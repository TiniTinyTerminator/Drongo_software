#include "spi.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

Spi::Spi(std::filesystem::path dev) : device_file(dev)
{
    fd = open(dev.c_str(), O_RDWR);

    if (fd < 0)
        throw std::runtime_error("Cannot open device");
}

Spi::~Spi()
{
    close(fd);
}

std::vector<char> Spi::receive(const unsigned int count)
{
    std::lock_guard guard(mtx);

    std::vector<char> rx(count);

    spi_ioc_transfer data =
    {
        .tx_buf = (unsigned long long)nullptr,
        .rx_buf = (unsigned long long)rx.data(),
        .len = count
    };
    
    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &data);

    if(ret < 0)
        throw std::runtime_error("Cannot receive spi data");

    return rx;
}


void Spi::transmit(std::vector<char> tx)
{
    std::lock_guard guard(mtx);

    spi_ioc_transfer data =
    {
        .tx_buf = (unsigned long long)tx.data(),
        .rx_buf = (unsigned long long)nullptr,
        .len = (unsigned int)tx.size()
    };
    
    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &data);

    if(ret < 0)
        throw std::runtime_error("Cannot send spi message");
}

std::vector<char> Spi::transceive(std::vector<char> tx)
{
    std::lock_guard guard(mtx);

    std::vector<char> rx(tx.size());

    spi_ioc_transfer data =
    {
        .tx_buf = (unsigned long long)tx.data(),
        .rx_buf = (unsigned long long)rx.data(),
        .len = (unsigned int)tx.size()
    };
    
    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &data);

    if(ret < 0)
        throw std::runtime_error("Cannot transceive spi message");

    return rx;
}

void Spi::set_speed(int speed)
{    
    std::lock_guard guard(mtx);

    int ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    if(ret < 0)
        throw std::runtime_error("Could write speed");
}

int Spi::get_speed(void) const
{
    int speed;
    int ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);

    if(ret < 0)
        throw std::runtime_error("Could read speed");

    return speed;
}

void Spi::set_mode(SpiModeConfig mode)
{
    std::lock_guard guard(mtx);

    int ret = ioctl(fd, SPI_IOC_WR_MODE, &mode.data);

    if(ret < 0)
        throw std::runtime_error("Could write mode");
}

SpiModeConfig Spi::get_mode(void) const
{
    SpiModeConfig mode;
    int ret = ioctl(fd, SPI_IOC_RD_MODE, &mode.data);

    if(ret < 0)
        throw std::runtime_error("Could read mode");

    return mode;
}

void Spi::set_lsb(bool lsb_first)
{
    std::lock_guard guard(mtx);

    int ret = ioctl(fd, SPI_IOC_WR_LSB_FIRST, &lsb_first);

    if(ret < 0)
        throw std::runtime_error("Could write LSB mode");}

bool Spi::get_lsb(void) const
{
    int lsb_first;
    int ret = ioctl(fd, SPI_IOC_RD_LSB_FIRST, &lsb_first);

    if(ret < 0)
        throw std::runtime_error("Could read LSB mode");

    return lsb_first;
}

void Spi::set_bits_per_word(int nbits)
{
    std::lock_guard guard(mtx);

    int ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &nbits);

    if(ret < 0)
        throw std::runtime_error("Could write bits per word");
}

int Spi::get_bits_per_word(void) const
{
    int nbits;
    int ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &nbits);

    if(ret < 0)
        throw std::runtime_error("Could read bits per word");

    return nbits;
}
