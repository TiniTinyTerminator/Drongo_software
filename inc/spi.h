/**
 * @file spi.h
 * @author Max Bensink (maxbensink@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2023-10-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SPI_H
#define SPI_H

#include <filesystem>
#include <string>
#include <vector>
#include <mutex>

/**
 * @brief Union representing SPI mode configuration.
 * 
 * This union allows easy access to individual SPI mode flags or the complete 16-bit configuration data.
 */
union SpiModeConfig
{
    struct
    {
        bool cpha : 1;           ///< Clock phase.
        bool cpol : 1;           ///< Clock polarity.
        bool cs_high : 1;        ///< Chip select active high.
        bool lsb_first : 1;      ///< LSB first.
        bool three_wire : 1;     ///< Three wire mode enabled.
        bool loop : 1;           ///< Loopback mode.
        bool no_cs : 1;          ///< No chip select.
        bool ready : 1;          ///< Ready flag.
        bool tx_dual : 1;        ///< Dual TX mode.
        bool tx_quad : 1;        ///< Quad TX mode.
        bool rx_dual : 1;        ///< Dual RX mode.
        bool rx_quad : 1;        ///< Quad RX mode.
        bool cs_word : 1;        ///< Word select for chip select.
        bool tx_octal : 1;       ///< Octal TX mode.
        bool rx_octal : 1;       ///< Octal RX mode.
        bool three_wire_hiz : 1; ///< Three wire mode with high impedance.
    } bits;                    ///< Bitfield structure for individual flag access.
    uint16_t data;             ///< Full 16-bit configuration data for direct access.
};

// Predefined SPI mode configurations
constexpr SpiModeConfig MODE_0 = {.data = (0 | 0)};       ///< Mode 0: CPOL=0, CPHA=0.
constexpr SpiModeConfig MODE_1 = {.data = (0 | 0x01)};    ///< Mode 1: CPOL=0, CPHA=1.
constexpr SpiModeConfig MODE_2 = {.data = (0x02 | 0)};    ///< Mode 2: CPOL=1, CPHA=0.
constexpr SpiModeConfig MODE_3 = {.data = (0x02 | 0x01)}; ///< Mode 3: CPOL=1, CPHA=1.

/**
 * @brief Class representing an SPI device.
 * 
 * This class provides functionalities to interact with an SPI device, such as transmitting and receiving data.
 */
class Spi
{
private:
    std::filesystem::path device_file; ///< Path to the SPI device file.
    int fd;                            ///< File descriptor for the SPI device.
    std::mutex mtx;                    ///< Mutex for thread-safe operations.

public:
    /**
     * @brief Constructor for the SPI class.
     * 
     * @param dev Path to the SPI device file.
     */
    Spi(std::filesystem::path dev);

    /**
     * @brief Destructor for the SPI class.
     */
    ~Spi();

    /**
     * @brief Receive data from SPI device.
     * 
     * @param length The number of characters to receive.
     * @return std::vector<char> The received data as a vector of characters.
     */
    std::vector<char> receive(const unsigned int length);

    /**
     * @brief Transmit data to SPI device.
     * 
     * @param data The data to be transmitted as a vector of characters.
     */
    void transmit(const std::vector<char> data);

    /**
     * @brief Transmit and receive data simultaneously.
     * 
     * @param data The data to be transmitted as a vector of characters.
     * @return std::vector<char> The received data as a vector of characters.
     */
    std::vector<char> transceive(const std::vector<char> data);

    /**
     * @brief Set the speed of the SPI communication.
     * 
     * @param speed The speed in Hz.
     */
    void set_speed(int speed);

    /**
     * @brief Get the current speed of the SPI communication.
     * 
     * @return int The current speed in Hz.
     */
    int get_speed(void) const;

    /**
     * @brief Set the SPI mode.
     * 
     * @param mode The SPI mode configuration.
     */
    void set_mode(SpiModeConfig mode);

    /**
     * @brief Get the current SPI mode.
     * 
     * @return SpiModeConfig The current SPI mode configuration.
     */
    SpiModeConfig get_mode(void) const;

    /**
     * @brief Set LSB (Least Significant Bit) first mode.
     * 
     * @param lsb If true, LSB first mode is enabled. If false, MSB (Most Significant Bit) first mode is enabled.
     */
    void set_lsb(bool lsb);

    /**
     * @brief Get the current LSB/MSB first mode setting.
     * 
     * @return bool True if LSB first mode is enabled, false if MSB first mode is enabled.
     */
    bool get_lsb(void) const;

    /**
     * @brief Set the number of bits per word in SPI communication.
     * 
     * @param bits The number of bits per word.
     */
    void set_bits_per_word(int bits);

    /**
     * @brief Get the current setting of bits per word in SPI communication.
     * 
     * @return int The current number of bits per word.
     */
    int get_bits_per_word(void) const;
};

#endif