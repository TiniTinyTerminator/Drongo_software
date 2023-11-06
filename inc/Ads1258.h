/**
 * @file Ads1258.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-10-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef ADS1258_H
#define ADS1258_H

#include <vector>
#include <map>
#include <cmath>

#include "spi.h"
#include "gpio.h"
#include "commands.h"

constexpr double ADC_MAX_VOLTAGE = 2.5;
constexpr double ADC_RAW_TO_DOUBLE_RATIO = ADC_MAX_VOLTAGE / (1 << 23);

enum AutoDataRates {
    AUTO_DRATE0 = 1831,
    AUTO_DRATE1 = 6168,
    AUTO_DRATE2 = 15123,
    AUTO_DRATE3 = 23739
};

enum FixedDataRates {
    FIXED_DRATE0 = 1953,
    FIXED_DRATE1 = 7813,
    FIXED_DRATE2 = 31250,
    FIXED_DRATE3 = 125000
};

enum DelaysInMicroseconds {
    DLY0 = 0,
    DLY1 = 8,
    DLY2 = 16,
    DLY3 = 32,
    DLY4 = 64,
    DLY5 = 128,
    DLY6 = 256,
    DLY7 = 384
};

typedef Muxsch FixedChannel;
typedef GpioReg GpioDirection;
typedef GpioReg GpioOutput;
typedef GpioReg GpioInput;

typedef std::pair<char, int32_t> ChannelData;

union SingleChannel
{
    struct {
        bool channel0 : 1;
        bool channel1 : 1;
        bool channel2 : 1;
        bool channel3 : 1;
        bool channel4 : 1;
        bool channel5 : 1;
        bool channel6 : 1;
        bool channel7 : 1;
        bool channel8 : 1;
        bool channel9 : 1;
        bool channel10 : 1;
        bool channel11 : 1;
        bool channel12 : 1;
        bool channel13 : 1;
        bool channel14 : 1;
        bool channel15 : 1;
    } channels;

    uint16_t raw_data;
};

union DiffChannel
{
    struct {
        bool channel0 : 1;
        bool channel1 : 1;
        bool channel2 : 1;
        bool channel3 : 1;
        bool channel4 : 1;
        bool channel5 : 1;
        bool channel6 : 1;
        bool channel7 : 1;
    } channels;

    uint16_t raw_data;
};

union SystemChannels
{
    struct {
        bool offset : 1;
        const bool zero1 : 1 = 0;
        bool vcc : 1;
        bool temprature : 1;
        bool gain : 1;
        bool reference : 1;
        const bool zero2 : 1 = 0;
        const bool zero3 : 1 = 0;

    } channels;

    uint16_t raw_data;
};

constexpr double channel_drate_delay_to_frequency(double n_channels, double drate, double delay_us)
{
    if(n_channels > 15)
        throw std::invalid_argument("number of channels cannot be bigger than 15");
    
    double time = 1 / drate + delay_us * 1E-6;

    return 1 / (time * n_channels);
}

class Ads1258 {
private:
    Spi _spi;
    Gpio _gpio;

    uint32_t _channels_active;
    uint8_t _n_channels_active;
    uint8_t _current_channel;
    
    std::map<RegisterAdressses, char> registers;

    void set_register(RegisterAdressses address, char data);
    void set_all_registers(void);
    char get_register(RegisterAdressses address);
    std::vector<char> get_all_registers(void);

    void reset_local_registers(void);
    void reset_channel_data(void);

public:
    Ads1258(std::filesystem::path spi, std::filesystem::path gpio);
    ~Ads1258();

    void start(bool start);
    void pwdn(bool pwdn);
    void reset(bool reset);

    void enable_auto(bool enable_auto);
    void enable_chop(bool enable_chop);
    void enable_quick_spi_reset(bool enable_rst);
    void enable_bypass(bool enable_bypass);
    void enable_external_clock(bool external_clk);
    void enable_status(bool enable_stats);
    void enable_sleep_mode(bool sleep_mode);

    void set_drate(DrateConfig drate);
    void set_scbcs(ScbcsConfig scbcs);
    void set_delay(DelayConfig delay);

    void set_fixed_channel(FixedChannel channel);
    void set_auto_single_channel(SingleChannel channels);
    void set_auto_diff_channel(DiffChannel channels);
    void set_system_readings(SystemChannels channels);
    
    void update_settings(void);
    bool verify_settings(void);

    void set_gpio_direction(GpioDirection channels);
    void set_gpio_output(GpioOutput outputs);
    GpioInput get_gpio_intput(void);
    IdReg get_id(void);

    bool await_data_ready(std::chrono::microseconds max_timeout);
    ChannelData get_data(void);
};

#endif