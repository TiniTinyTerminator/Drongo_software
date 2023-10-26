
#include <gpiod.h>
#include <filesystem>
#include <chrono>
#include <map>

/**
 * @brief possible in- and output values of a gpio pin
 * 
 */
enum class Values {
    LOW = 0x0,
    HIGH
};

/**
 * @brief possible directions of a gpio pin
 * 
 */
enum class Direction {
    INPUT = 0x0,
    OUTPUT
};

/**
 * @brief types of edge detection for gpio pin
 * 
 */
enum class Detection {
    RISING = 0x0,
    FALLING,
    BOTH
};

/**
 * @brief class for handling gpio pins through an easy interface interface
 * 
 */
class Gpio
{
private:
    std::filesystem::path device_file;

    std::string label;

    struct gpiod_chip *chip;
    
    inline gpiod_line *retrieve_gpiod_line(int line_id);

public:
    Gpio(std::filesystem::path dev);
    ~Gpio();

    void setup();
    void set_direction(int line_id, Direction direction);
    void set_output(int line_id, Values value);
    Values get_input(int line_id);
    
    void set_detection(int line_id, Detection detection);
    void get_event_info(int line_id);
    bool wait_for_event(int line_id, std::chrono::nanoseconds timeout);

    void release_line(int line_id);

};

