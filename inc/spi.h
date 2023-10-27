#include <filesystem>
#include <string>
#include <vector>
#include <mutex>

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

    void set_mode(int);
    int get_mode(void) const;

    void set_lsb(bool);
    bool get_lsb(void) const;

    void set_bits_per_word(int);
    int get_bits_per_word(void) const;

};
