/**
 * @file Ads1258.cpp
 * @author Max Bensink (maxbensink@outlook.com)
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

#include "easylogging++.h"

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

Ads1258::Ads1258(std::filesystem::path spi, std::filesystem::path gpio) : _spi(spi), _gpio(gpio), _channels_active(0x0)
{
    _spi.set_mode(MODE_3);
    _spi.set_speed(12e6);
    _spi.set_bits_per_word(8);

    _gpio.set_direction(Pins::CLKSEL, Direction::OUTPUT);
    _gpio.set_direction(Pins::START, Direction::OUTPUT);
    _gpio.set_direction(Pins::RST, Direction::OUTPUT);
    _gpio.set_direction(Pins::PWDN, Direction::OUTPUT);

    _gpio.set_output(Pins::CLKSEL, Values::LOW);
    _gpio.set_output(Pins::START, Values::LOW);
    _gpio.set_output(Pins::RST, Values::LOW);
    _gpio.set_output(Pins::PWDN, Values::LOW);

    // _gpio.set_detection(Pins::DRDY, Detection::FALLING);
    _gpio.set_timeout(5us);

    reset_local_registers();
}

void Ads1258::reset_local_registers(void)
{
    registers[RegisterAdressses::CONFIG0] = CONFIG0_DEFAULT.raw_data;
    registers[RegisterAdressses::CONFIG1] = CONFIG1_DEFAULT.raw_data;
    registers[RegisterAdressses::MUXSCH] = MUXSCH_DEFAULT.raw_data;
    registers[RegisterAdressses::MUXDIF] = MUXDIF_DEFAULT.raw_data;
    registers[RegisterAdressses::MUXSG0] = MUXSG0_DEFAULT.raw_data;
    registers[RegisterAdressses::MUXSG1] = MUXSG1_DEFAULT.raw_data;
    registers[RegisterAdressses::GPIOC] = GPIOC_DEFAULT.raw_data;
    registers[RegisterAdressses::GPIOD] = GPIOD_DEFAULT.raw_data;
    registers[RegisterAdressses::SYSRED] = SYSRED_DEFAULT.raw_data;
}

void Ads1258::reset_channel_data(void)
{
    _channels_active = 0;
    _current_channel = 0;
    _n_channels_active = 0;
}

Ads1258::~Ads1258()
{
    _gpio.set_output(Pins::CLKSEL, Values::LOW);
    _gpio.set_output(Pins::START, Values::LOW);
    _gpio.set_output(Pins::RST, Values::LOW);
    _gpio.set_output(Pins::PWDN, Values::LOW);
}

void Ads1258::start(bool start)
{
    _gpio.set_output(Pins::START, start ? HIGH : LOW);

    if (_gpio.get_input(Pins::START) == LOW)
    {
        reset_channel_data();
    }
}

void Ads1258::pwdn(bool pwdn)
{
    _gpio.set_output(Pins::PWDN, pwdn ? LOW : HIGH);

    _current_channel = 0 ? _gpio.get_input(Pins::PWDN) == HIGH : _current_channel;

    if (_gpio.get_input(Pins::PWDN) == HIGH)
    {
        reset_local_registers();
        reset_channel_data();
        _channel_ids = {};
    }
}

void Ads1258::reset(bool reset)
{
    _gpio.set_output(Pins::RST, reset ? LOW : HIGH);

    if (_gpio.get_input(Pins::RST) == LOW)
    {
        reset_local_registers();
        reset_channel_data();
        _channel_ids = {};
    }
}

void Ads1258::set_register(RegisterAdressses address, char data)
{
    const CommandByte command = {.bits = {address, false, Commands::WRITE_REGISTERS}};

    std::vector<char> message = {command.raw_data, data};

    _spi.transmit(message);

    _current_channel = 0;
}

void Ads1258::set_all_registers(void)
{
    constexpr CommandByte command = {.bits = {0x0, true, Commands::WRITE_REGISTERS}};

    std::vector<char> message(NUM_REGISTERS - 1 - 1);
    message[0] = command.raw_data;

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

    message[0] = command.raw_data;

    return _spi.transceive(message)[1];
}

std::vector<char> Ads1258::get_all_registers(void)
{
    const CommandByte command = {.bits = {0x0, true, Commands::READ_REGISTERS}};

    std::vector<char> message(NUM_REGISTERS + 1);

    message[0] = command.raw_data;

    return _spi.transceive(message);
}

void Ads1258::enable_auto(bool enable_auto)
{
    Config0 cf0 = {.raw_data = registers[RegisterAdressses::CONFIG0]};

    if (cf0.bits.muxmod == !enable_auto)
        return;

    cf0.bits.muxmod = !enable_auto;

    set_register(RegisterAdressses::CONFIG0, cf0.raw_data);

    registers[RegisterAdressses::CONFIG0] = cf0.raw_data;
}

void Ads1258::enable_chop(bool enable_chop)
{
    Config0 cf0 = {.raw_data = registers[RegisterAdressses::CONFIG0]};

    if (cf0.bits.chop == enable_chop)
        return;

    cf0.bits.chop = enable_chop;

    set_register(RegisterAdressses::CONFIG0, cf0.raw_data);

    registers[RegisterAdressses::CONFIG0] = cf0.raw_data;
}

void Ads1258::enable_quick_spi_reset(bool enable_rst)
{
    Config0 cf0 = {.raw_data = registers[RegisterAdressses::CONFIG0]};

    if (cf0.bits.spirst == enable_rst)
        return;

    cf0.bits.spirst = enable_rst;

    set_register(RegisterAdressses::CONFIG0, cf0.raw_data);

    registers[RegisterAdressses::CONFIG0] = cf0.raw_data;
}

void Ads1258::enable_bypass(bool enable_bypass)
{
    Config0 cf0 = {.raw_data = registers[RegisterAdressses::CONFIG0]};

    if (cf0.bits.bypass == enable_bypass)
        return;

    cf0.bits.bypass = enable_bypass;

    set_register(RegisterAdressses::CONFIG0, cf0.raw_data);

    registers[RegisterAdressses::CONFIG0] = cf0.raw_data;
}

void Ads1258::enable_external_clock(bool external_clk)
{
    Config0 cf0 = {.raw_data = registers[RegisterAdressses::CONFIG0]};

    if (cf0.bits.clken == external_clk)
        return;

    cf0.bits.clken = external_clk;

    set_register(RegisterAdressses::CONFIG0, cf0.raw_data);

    registers[RegisterAdressses::CONFIG0] = cf0.raw_data;
}

void Ads1258::enable_status(bool enable_stats)
{
    Config0 cf0 = {.raw_data = registers[RegisterAdressses::CONFIG0]};

    if (cf0.bits.stat == enable_stats)
        return;

    cf0.bits.stat = enable_stats;

    set_register(RegisterAdressses::CONFIG0, cf0.raw_data);

    registers[RegisterAdressses::CONFIG0] = cf0.raw_data;
}

void Ads1258::enable_sleep_mode(bool sleep_mode)
{
    Config1 cf1 = {.raw_data = registers[RegisterAdressses::CONFIG1]};

    if (cf1.bits.idle_mode == sleep_mode)
        return;

    cf1.bits.idle_mode = sleep_mode;

    set_register(RegisterAdressses::CONFIG1, cf1.raw_data);

    registers[RegisterAdressses::CONFIG1] = cf1.raw_data;
}

void Ads1258::set_drate(uint32_t drate)
{
    Config1 cf1 = {.raw_data = registers[RegisterAdressses::CONFIG1]};

    if (cf1.bits.data_rate == drate)
        return;

    cf1.bits.data_rate = drate;

    set_register(RegisterAdressses::CONFIG1, cf1.raw_data);

    registers[RegisterAdressses::CONFIG1] = cf1.raw_data;
}

void Ads1258::set_scbcs(ScbcsConfig scbcs)
{
    Config1 cf1 = {.raw_data = registers[RegisterAdressses::CONFIG1]};

    if (cf1.bits.scbcs == scbcs)
        return;

    cf1.bits.scbcs = scbcs;

    set_register(RegisterAdressses::CONFIG1, cf1.raw_data);

    registers[RegisterAdressses::CONFIG1] = cf1.raw_data;
}

void Ads1258::set_delay(uint32_t delay)
{
    Config1 cf1 = {.raw_data = registers[RegisterAdressses::CONFIG1]};

    if (cf1.bits.delay == delay)
        return;

    cf1.bits.delay = delay;

    set_register(RegisterAdressses::CONFIG1, cf1.raw_data);

    registers[RegisterAdressses::CONFIG1] = cf1.raw_data;
}

void Ads1258::set_fixed_channel(FixedChannel channel)
{
    registers[RegisterAdressses::MUXSCH] = channel.raw_data;

    set_register(RegisterAdressses::MUXSCH, channel.raw_data);
}

void Ads1258::set_auto_single_channel(SingleChannel channels)
{
    registers[RegisterAdressses::MUXSG0] = channels.raw_data;
    registers[RegisterAdressses::MUXSG1] = channels.raw_data >> 8;

    _channels_active = ((uint32_t)(channels.raw_data) << 8) | (~0xFFFF & _channels_active);

    _n_channels_active = count_set_bits(_channels_active);

    set_register(RegisterAdressses::MUXSG0, channels.raw_data);
    set_register(RegisterAdressses::MUXSG1, channels.raw_data >> 8);

    _channel_ids = get_active_channels();
}

void Ads1258::set_auto_diff_channel(DiffChannel channels)
{
    registers[RegisterAdressses::MUXDIF] = channels.raw_data;

    _channels_active = channels.raw_data | (~0xFF & _channels_active);

    _n_channels_active = count_set_bits(_channels_active);

    set_register(RegisterAdressses::MUXDIF, channels.raw_data);

    _channel_ids = get_active_channels();
}

void Ads1258::set_system_readings(SystemChannels channels)
{
    registers[RegisterAdressses::SYSRED] = channels.raw_data;

    _channels_active = channels.channels.offset << 24 | (~(0x1 << 24) & _channels_active);
    _channels_active = channels.raw_data << (25 - 2) | (~(0b1111 << 25) & _channels_active);

    _n_channels_active = count_set_bits(_channels_active);

    set_register(RegisterAdressses::SYSRED, channels.raw_data);

    _channel_ids = get_active_channels();
}

void Ads1258::set_gpio_direction(GpioDirection channels)
{
    registers[RegisterAdressses::GPIOC] = channels.raw_data;

    set_register(RegisterAdressses::GPIOC, channels.raw_data);
}

void Ads1258::set_gpio_output(GpioOutput outputs)
{
    registers[RegisterAdressses::GPIOD] = outputs.raw_data & ~registers[RegisterAdressses::GPIOC];

    set_register(RegisterAdressses::GPIOD, outputs.raw_data);
}

GpioInput Ads1258::get_gpio_input(void)
{
    return {.raw_data = static_cast<char>((get_register(RegisterAdressses::GPIOD) & registers[RegisterAdressses::GPIOC]))};
}

bool Ads1258::verify_settings(void)
{
    std::vector<char> adc_data = get_all_registers();

    for (auto reg : registers)
    {
        if (adc_data[reg.first + 1] != reg.second)
            return false;
    }

    return true;
}

void Ads1258::update_settings(void)
{
    set_all_registers();
}

IdReg Ads1258::get_id(void)
{
    return {.raw_data = get_register(RegisterAdressses::ID)};
}

std::pair<ChannelData, ChannelData> Ads1258::get_data_read(void)
{

    constexpr CommandByte command = {.bits = {0x0, true, Commands::READ_COMMAND}};

    ChannelData data_1, data_2;

    std::vector<char> rx = _spi.transceive({command.raw_data, 0x0, 0x0, 0x0, 0x0, command.raw_data, 0x0, 0x0, 0x0, 0x0});

    StatusByte stats = {.raw_data = rx[1]};

    data_1 = {static_cast<char>(stats.bits.CHID), static_cast<int32_t>((uint32_t)rx[4] << 8 | ((uint32_t)rx[3] << 16) | ((uint32_t)(rx[2]) << 24)) >> 8};

    stats = {.raw_data = rx[6]};

    data_2 = {static_cast<char>(stats.bits.CHID), static_cast<int32_t>((uint32_t)rx[9] << 8 | ((uint32_t)rx[8] << 16) | ((uint32_t)(rx[7]) << 24)) >> 8};

    _current_channel = data_1.first;

    return {data_1, data_2};
}

ChannelData Ads1258::get_data_direct(void)
{
    const Config0 cf0 = {.raw_data = registers[RegisterAdressses::CONFIG0]};

    constexpr CommandByte command = {.bits = {0x0, true, Commands::READ_DIRECT}};

    std::vector<char> rx = _spi.receive(cf0.bits.stat ? 4 : 3);

    if (cf0.bits.stat)
    {
        StatusByte stats = {.raw_data = rx[0]};

        int32_t value = static_cast<int32_t>((uint32_t)rx[3] << 8 | ((uint32_t)rx[2] << 16) | ((uint32_t)(rx[1]) << 24));

        value >>= 8;

        return {static_cast<char>(stats.bits.CHID), value};
    }
    else
    {

        int32_t value = static_cast<int32_t>((uint32_t)rx[2] << 8 | ((uint32_t)rx[1] << 16) | ((uint32_t)(rx[0]) << 24));

        value >>= 8;

        return {0, value};
    }
}

// bool Ads1258::await_data_ready(void)
// {
//     return _gpio.wait_for_event(Pins::DRDY);
// }

std::vector<uint8_t> Ads1258::get_active_channels(void)
{
    uint8_t index = 0;

    std::vector<uint8_t> channels;

    for (uint8_t i = 0; i < 32; i++)
    {
        if (_channels_active & (1 << i))
            channels.push_back(i);
    }

    return channels;
}

uint8_t Ads1258::get_current_channel(void)
{
    return _current_channel;
}
