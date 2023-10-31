/**
 * @file Ads1258.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-10-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <iostream>
#include <bitset>
#include <thread>

#include "spi.h"
#include "RaspberryPiGPIO.h"

#include "Ads1258.h"

using namespace RaspberryPi;
using namespace std::chrono_literals;
enum Pins
{
    CLKSEL = PhysicalToBCM::PIN11,
    RST = PhysicalToBCM::PIN12,
    DRDY = PhysicalToBCM::PIN13,
    START = PhysicalToBCM::PIN15,
    PWDN = PhysicalToBCM::PIN16
};

int count_set_bits(int n)
{
    int count = 0;
    while (n)
    {
        n &= n - 1; // unset the rightmost set bit
        count++;
    }
    return count;
}

template <class T = char ,int shift>
constexpr T shifted_byte_repair(T first, T second)
{
    static_assert(std::is_integral<T>::value, "Type T must be integral");

    constexpr int8_t max_shift = sizeof(T) * 8;

    static_assert(shift >= -max_shift && shift <= max_shift, "Shift value out of allowed range!");

    constexpr T lrem_mask = T(1 << max_shift) - 1 >> abs(shift);
    constexpr T rrem_mask = T(1 << max_shift) - 1 << abs(shift);

    if (shift < 0)
    {
        return (first & lrem_mask) << abs(shift) | (second & rrem_mask);
    }
    else
    {
        return (first & lrem_mask) | (second & rrem_mask) >> abs(shift);
    }
}

union RawToInteger
{
    struct
    {
        char byte1 : 8;
        char byte2 : 8;
        char byte3 : 8;
        char byte4 : 8;
    } bits;

    int integer;
};

Ads1258::Ads1258(std::filesystem::path spi, std::filesystem::path gpio) : _spi(spi), _gpio(gpio), channel_active(0x0)
{
    _spi.set_mode(MODE_0);
    _spi.set_speed(31.2e6);
    _spi.set_bits_per_word(8);

    _gpio.set_direction(Pins::CLKSEL, Direction::OUTPUT);
    _gpio.set_direction(Pins::START, Direction::OUTPUT);
    _gpio.set_direction(Pins::RST, Direction::OUTPUT);
    _gpio.set_direction(Pins::PWDN, Direction::OUTPUT);

    _gpio.set_output(Pins::CLKSEL, Values::LOW);

    _gpio.set_detection(Pins::DRDY, Detection::FALLING);

    registers[RegisterAdressses::CONFIG0] = CONFIG0_DEFAULT.data;
    registers[RegisterAdressses::CONFIG1] = CONFIG1_DEFAULT.data;
    registers[RegisterAdressses::MUXSCH] = MUXSCH_DEFAULT.data;
    registers[RegisterAdressses::MUXDIF] = MUXDIF_DEFAULT.data;
    registers[RegisterAdressses::MUXSG0] = MUXSG0_DEFAULT.data;
    registers[RegisterAdressses::MUXSG1] = MUXSG1_DEFAULT.data;
    registers[RegisterAdressses::GPIOC] = GPIOC_DEFAULT.data;
    registers[RegisterAdressses::GPIOD] = GPIOD_DEFAULT.data;
    registers[RegisterAdressses::SYSRED] = SYSRED_DEFAULT.data;
}

Ads1258::~Ads1258()
{
}

void Ads1258::start(bool start)
{
    _gpio.set_output(Pins::START, start ? HIGH : LOW);
}

void Ads1258::pwdn(bool pwdn)
{
    _gpio.set_output(Pins::PWDN, pwdn ? LOW : HIGH);
}

void Ads1258::reset(bool reset)
{
    _gpio.set_output(Pins::RST, reset ? LOW : HIGH);
}

void Ads1258::set_register(RegisterAdressses address, char data)
{
    const CommandByte command = {.bits = {address, false, Commands::WRITE_REGISTERS}};

    std::vector<char> message = {command.data, data};

    _spi.transmit(message);
}

void Ads1258::set_all_registers(void)
{
    constexpr CommandByte command = {.bits = {0x0, true, Commands::WRITE_REGISTERS}};

    std::vector<char> message(NUM_REGISTERS - 1 - 1);
    message[0] = command.data;

    for (auto reg_data : registers)
    {
        message[reg_data.first + 1] = reg_data.second;
    }

    _spi.transmit(message);
}

char Ads1258::get_register(RegisterAdressses address)
{
    const CommandByte command = {.bits = {address, false, Commands::READ_REGISTERS}};

    std::vector<char> message(2);

    message[0] = command.data;

    return _spi.transceive(message)[1];
}

std::vector<char> Ads1258::get_all_registers(void)
{
    const CommandByte command = {.bits = {0x0, true, Commands::READ_REGISTERS}};

    std::vector<char> message(NUM_REGISTERS + 1);

    message[0] = command.data;

    return _spi.transceive(message);
}

void Ads1258::enable_auto(bool enable_auto)
{
    Config0 cf0 = {.data = registers[RegisterAdressses::CONFIG0]};

    if (cf0.bits.muxmod == !enable_auto)
        return;

    cf0.bits.muxmod = !enable_auto;

    set_register(RegisterAdressses::CONFIG0, cf0.data);

    registers[RegisterAdressses::CONFIG0] = cf0.data;
}

void Ads1258::enable_chop(bool enable_chop)
{
    Config0 cf0 = {.data = registers[RegisterAdressses::CONFIG0]};

    if (cf0.bits.chop == enable_chop)
        return;

    cf0.bits.chop = enable_chop;

    set_register(RegisterAdressses::CONFIG0, cf0.data);

    registers[RegisterAdressses::CONFIG0] = cf0.data;
}

void Ads1258::enable_quick_spi_reset(bool enable_rst)
{
    Config0 cf0 = {.data = registers[RegisterAdressses::CONFIG0]};

    if (cf0.bits.spirst == enable_rst)
        return;

    cf0.bits.spirst = enable_rst;

    set_register(RegisterAdressses::CONFIG0, cf0.data);

    registers[RegisterAdressses::CONFIG0] = cf0.data;
}

void Ads1258::enable_bypass(bool enable_bypass)
{
    Config0 cf0 = {.data = registers[RegisterAdressses::CONFIG0]};

    if (cf0.bits.bypass == enable_bypass)
        return;

    cf0.bits.bypass = enable_bypass;

    set_register(RegisterAdressses::CONFIG0, cf0.data);

    registers[RegisterAdressses::CONFIG0] = cf0.data;
}

void Ads1258::enable_external_clock(bool external_clk)
{
    Config0 cf0 = {.data = registers[RegisterAdressses::CONFIG0]};

    if (cf0.bits.clken == external_clk)
        return;

    cf0.bits.clken = external_clk;

    set_register(RegisterAdressses::CONFIG0, cf0.data);

    registers[RegisterAdressses::CONFIG0] = cf0.data;
}

void Ads1258::enable_status(bool enable_stats)
{
    Config0 cf0 = {.data = registers[RegisterAdressses::CONFIG0]};

    if (cf0.bits.stat == enable_stats)
        return;

    cf0.bits.stat = enable_stats;

    set_register(RegisterAdressses::CONFIG0, cf0.data);

    registers[RegisterAdressses::CONFIG0] = cf0.data;
}

void Ads1258::enable_sleep_mode(bool sleep_mode)
{
    Config1 cf1 = {.data = registers[RegisterAdressses::CONFIG1]};

    if (cf1.bits.idle_mode == sleep_mode)
        return;

    cf1.bits.idle_mode = sleep_mode;

    set_register(RegisterAdressses::CONFIG1, cf1.data);

    registers[RegisterAdressses::CONFIG1] = cf1.data;
}

void Ads1258::set_drate(DrateConfig drate)
{
    Config1 cf1 = {.data = registers[RegisterAdressses::CONFIG1]};

    if (cf1.bits.data_rate == drate)
        return;

    cf1.bits.data_rate = drate;

    set_register(RegisterAdressses::CONFIG1, cf1.data);

    registers[RegisterAdressses::CONFIG1] = cf1.data;
}

void Ads1258::set_scbcs(ScbcsConfig scbcs)
{
    Config1 cf1 = {.data = registers[RegisterAdressses::CONFIG1]};

    if (cf1.bits.scbcs == scbcs)
        return;

    cf1.bits.scbcs = scbcs;

    set_register(RegisterAdressses::CONFIG1, cf1.data);

    registers[RegisterAdressses::CONFIG1] = cf1.data;
}

void Ads1258::set_delay(DelayConfig delay)
{
    Config1 cf1 = {.data = registers[RegisterAdressses::CONFIG1]};

    if (cf1.bits.delay == delay)
        return;

    cf1.bits.delay = delay;

    set_register(RegisterAdressses::CONFIG1, cf1.data);

    registers[RegisterAdressses::CONFIG1] = cf1.data;
}

void Ads1258::set_fixed_channel(FixedChannel channel)
{
    registers[RegisterAdressses::MUXSCH] = channel.data;

    set_register(RegisterAdressses::MUXSCH, channel.data);
}

void Ads1258::set_auto_single_channel(SingleChannel channels)
{
    registers[RegisterAdressses::MUXSG0] = channels.data;
    registers[RegisterAdressses::MUXSG1] = channels.data >> 8;

    channel_active = ((uint32_t)(channels.data) << 8) | (~0xFFFF & channel_active);

    n_channels_active = count_set_bits(channel_active);

    set_register(RegisterAdressses::MUXSG0, channels.data);
    set_register(RegisterAdressses::MUXSG1, channels.data >> 8);
}

void Ads1258::set_auto_diff_channel(DiffChannel channels)
{
    registers[RegisterAdressses::MUXDIF] = channels.data;

    channel_active = channels.data | (~0xFF & channel_active);

    n_channels_active = count_set_bits(channel_active);

    set_register(RegisterAdressses::MUXDIF, channels.data);
}

void Ads1258::set_system_readings(SystemChannels channels)
{
    registers[RegisterAdressses::SYSRED] = channels.data;

    channel_active = channels.channels.offset << 24 | (~(0x1 << 24) & channel_active);
    channel_active = channels.data << (25 - 2) | (~(0b1111 << 25) & channel_active);

    n_channels_active = count_set_bits(channel_active);

    set_register(RegisterAdressses::SYSRED, channels.data);
}

void Ads1258::set_gpio_direction(GpioDirection channels)
{
    registers[RegisterAdressses::GPIOC] = channels.data;

    set_register(RegisterAdressses::GPIOC, channels.data);
}

void Ads1258::set_gpio_output(GpioOutput outputs)
{
    registers[RegisterAdressses::GPIOD] = outputs.data & ~registers[RegisterAdressses::GPIOC];

    set_register(RegisterAdressses::GPIOD, outputs.data);
}

GpioInput Ads1258::get_gpio_intput(void)
{
    return {.data = static_cast<char>((get_register(RegisterAdressses::GPIOD) & registers[RegisterAdressses::GPIOC]))};
}

Id Ads1258::get_id(void)
{
    return {.data = get_register(RegisterAdressses::ID)};
}

std::vector<uint32_t> Ads1258::get_data(void)
{
    Config0 cf0 = {.data = registers[RegisterAdressses::CONFIG0]};

    RawToInteger conversion_struct;
    conversion_struct.bits.byte4 = 0x0;

    if (cf0.bits.muxmod)
    {
        std::vector<char> data = _spi.receive(4);

        conversion_struct.bits.byte3 = data[2];
        conversion_struct.bits.byte2 = data[1];
        conversion_struct.bits.byte1 = data[0];

        return {(uint32_t)conversion_struct.integer};
    }
    else
    {
        std::vector<uint32_t> data;

        CommandByte command = {.bits = {.address = 0x0, .multiple=true, .command = Commands::READ_COMMAND}};

        std::vector<std::vector<char>> something;

        std::cout << std::bitset<32>(channel_active) << std::endl;

        for(uint8_t i = 0; i < 31; i++)
        {

            if((channel_active & (1 << i)) == 0x0) continue;

            std::cout << std::bitset<32>((1 << i)) << std::endl;

            // command.bits.address = i;
            something.push_back(_spi.transceive({command.data, 0x0, 0x0, 0x0, 0x0, 0x0}));

            std::this_thread::sleep_for(100us);

        }

        for(int i = 0; i < something.size(); i++)
        {
            volatile StatusByte status = {.data = something[i][1]};

            std::cout << std::bitset<32>((1 << status.data)) << std::endl;
        }

        return data;

    }
}

bool Ads1258::await_data_ready(std::chrono::microseconds max_timeout)
{
    return _gpio.wait_for_event(Pins::DRDY, max_timeout);
}
