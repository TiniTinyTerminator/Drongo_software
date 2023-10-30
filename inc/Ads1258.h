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

#include "spi.h"
#include "gpio.h"

#include "commands.h"



typedef Muxsch FixedChannel;
typedef GpioReg GpioDirection;
typedef GpioReg GpioOutput;
typedef GpioReg GpioInput;

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

    uint16_t data;
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

    uint16_t data;
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

    uint16_t data;
};

class Ads1258 {
private:
    Spi _spi;
    Gpio _gpio;

    uint32_t channel_active;
    uint8_t n_channels_active;

    std::map<RegisterAdressses, char> registers;

    void set_register(RegisterAdressses address, char data);
    void set_all_registers(void);
    char get_register(RegisterAdressses address);
    std::vector<char> get_all_registers(void);
    
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

    void set_gpio_direction(GpioDirection channels);
    void set_gpio_output(GpioOutput outputs);
    GpioInput get_gpio_intput(void);
    Id get_id(void);

    std::vector<uint32_t> get_data(void);
};

#endif