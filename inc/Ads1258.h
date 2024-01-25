/**
 * @file Ads1258.h
 * @author Max Bensink (maxbensink@outlook.com)
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

enum AutoDataRates
{
    AUTO_DRATE0 = 1831,
    AUTO_DRATE1 = 6168,
    AUTO_DRATE2 = 15123,
    AUTO_DRATE3 = 23739
};

enum FixedDataRates
{
    FIXED_DRATE0 = 1953,
    FIXED_DRATE1 = 7813,
    FIXED_DRATE2 = 31250,
    FIXED_DRATE3 = 125000
};

enum DelaysInMicroseconds
{
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

typedef std::pair<uint8_t, int32_t> ChannelData;

union SingleChannel
{
    struct
    {
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
    struct
    {
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
    struct
    {
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

constexpr double channel_drate_delay_to_frequency(const double n_channels, const double drate, const double delay_us)
{
    double time = 1 / drate + delay_us * 1e-6;

    return 1 / (time * n_channels);
}

constexpr double drate_delay_to_frequency(const double drate, const double delay_us)
{
    double time = 1 / drate + delay_us * 1e-6;

    return 1 / time;
}

class Ads1258
{
private:
    Spi _spi;
    Gpio _gpio;

    uint32_t _channels_active;
    uint8_t _n_channels_active;
    uint8_t _current_channel;

    uint8_t _channel_index;
    std::vector<uint8_t> _channel_ids;

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

    /**
     * @brief control the ADCs' start pin
     *
     * @param start true == on and false == off
     */
    void start(bool start);

    /**
     * @brief control the ADCs' pwdn pin
     *
     * @param pwdn true == on and false == off
     */
    void pwdn(bool pwdn);

    /**
     * @brief control the ADCs' reset pin
     *
     * @param reset true == on and false == off
     */
    void reset(bool reset);

    /**
     * @brief enable the auto select option of the adc
     *
     * @param enable_auto to turn on or off automode
     */
    void enable_auto(bool enable_auto);

    /**
     * @brief enable the chop option of the adc
     *
     * @param enable_auto to turn on or off the chop
     */
    void enable_chop(bool enable_chop);

    /**
     * @brief enable quick reset on the SPI bus
     *
     * @param enable_auto to enable/disable quick reset
     */
    void enable_quick_spi_reset(bool enable_rst);

    /**
     * @brief enable bypass to external electrical condioning circuit
     *
     * @param enable_auto to enable/disable quick reset
     */
    void enable_bypass(bool enable_bypass);

    /**
     * @brief enable external clock
     *
     * @param external_clk to enable/disable external clock
     */
    void enable_external_clock(bool external_clk);

    /**
     * @brief enable status byte
     *
     * @param status to enable/disable status byte with direct data request
     */
    void enable_status(bool enable_stats);

    /**
     * @brief enable sleep mode when start pin is low
     *
     * @param sleep_mode to enable/disable sleep mode
     */
    void enable_sleep_mode(bool sleep_mode);

    /**
     * @brief Set the data rate of ADC
     *
     * @param drate can either be 0, 1, 2, or 3
     */
    void set_drate(uint32_t drate);

    /**
     * @brief set bias current source
     *
     * @param drate see ScbcsConfig for more details
     */
    void set_scbcs(ScbcsConfig scbcs);

    /**
     * @brief Set delay of rate of ADC
     *
     * @param drate can be 0 to 7 depending on the desired delay
     */
    void set_delay(uint32_t delay);

    /**
     * @brief enable specific fixed channels
     *
     * @param channel byte where each bit represents a data entrypoint
     */
    void set_fixed_channel(FixedChannel channel);

    /**
     * @brief enable specific automode single ended channels
     *
     * @param channel short where each bit represents a channel
     */
    void set_auto_single_channel(SingleChannel channels);

    /**
     * @brief enable specific automode differential channels
     *
     * @param channel byte where each bit represents a channel
     */
    void set_auto_diff_channel(DiffChannel channels);

    /**
     * @brief enable specific system data channels
     *
     * @param channel byte where each bit represents a channel
     */
    void set_system_readings(SystemChannels channels);

    /**
     * @brief update settings on ADC
     * 
     */
    void update_settings(void);

    /**
     * @brief retrieve settings from ADC to verify configuration
     * 
     * @return true if settings are correct with what is saved locally
     * @return false if settings are incorrect with what is saved locally
     */
    bool verify_settings(void);

    /**
     * @brief Set the gpio direction per channel (as bitset)
     * 
     * @param channels byte to configure channels with
     */
    void set_gpio_direction(GpioDirection channels);

    /**
     * @brief Set the gpio output high or low
     * 
     * @param outputs byte to configure high or low per channel with
     */
    void set_gpio_output(GpioOutput outputs);

    /**
     * @brief Get the gpio intput byte
     * 
     * @return GpioInput byte of input data
     */
    GpioInput get_gpio_input(void);

    /**
     * @brief Get the id of ADC
     * 
     * @return ADC value
     */
    IdReg get_id(void);

    /**
     * @brief Get 2 pairs of channeldata
     * 
     * @return std::pair<ChannelData, ChannelData> 2 pairs of adc data
     */
    std::pair<ChannelData, ChannelData> get_data_read(void);

    /**
     * @brief Get the data using data direct command
     * 
     * @return ChannelData channeldata
     */
    ChannelData get_data_direct(void);

    /**
     * @brief Get the active channels
     * 
     * @return std::vector<uint8_t> active channels
     */
    std::vector<uint8_t> get_active_channels(void);

    /**
     * @brief Get the current channel
     * 
     * @return uint8_t current channel id
     */
    uint8_t get_current_channel(void);
};

#endif