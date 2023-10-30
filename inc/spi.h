/**
 * @file spi.h
 * @author your name (you@domain.com)
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

union SpiModeConfig
{
    struct
    {
        bool cpha : 1;           // 0x01
        bool cpol : 1;           // 0x02
        bool cs_high : 1;        // 0x04
        bool lsb_first : 1;      // 0x08
        bool three_wire : 1;     // 0x10
        bool loop : 1;           // 0x20
        bool no_cs : 1;          // 0x40
        bool ready : 1;          // 0x80
        bool tx_dual : 1;        // 0x100
        bool tx_quad : 1;        // 0x200
        bool rx_dual : 1;        // 0x400
        bool rx_quad : 1;        // 0x800
        bool cs_word : 1;        // 0x1000
        bool tx_octal : 1;       // 0x2000
        bool rx_octal : 1;       // 0x4000
        bool three_wire_hiz : 1; // 0x8000
    } bits;
    uint16_t data; // The full 16 bits for direct access
};

constexpr SpiModeConfig MODE_0 = {.data = (0 | 0)};
constexpr SpiModeConfig MODE_1 = {.data = (0 | 0x01)};
constexpr SpiModeConfig MODE_2 = {.data = (0x02 | 0)};
constexpr SpiModeConfig MODE_3 = {.data = (0x02 | 0x01)};

class Spi
{
private:
    std::filesystem::path device_file;

    int fd;

    std::mutex mtx;

public:
    Spi(std::filesystem::path dev);
    ~Spi();

    std::vector<char> receive(const unsigned int);
    void transmit(const std::vector<char>);

    std::vector<char> transceive(const std::vector<char>);

    void set_speed(int);
    int get_speed(void) const;

    void set_mode(SpiModeConfig);
    SpiModeConfig get_mode(void) const;

    void set_lsb(bool);
    bool get_lsb(void) const;

    void set_bits_per_word(int);
    int get_bits_per_word(void) const;
};

#endif