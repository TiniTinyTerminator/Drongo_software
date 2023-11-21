/**
 * @file gpio.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-10-30
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef GPIO_H
#define GPIO_H

#include <gpiod.h>
#include <filesystem>
#include <chrono>
#include <map>

/**
 * @brief possible in- and output values of a gpio pin
 *
 */
enum Values : int
{
    LOW = 0x0,
    HIGH
};

/**
 * @brief possible directions of a gpio pin
 *
 */
enum Direction : int
{
    INPUT = 0x0,
    OUTPUT
};

/**
 * @brief types of edge detection for gpio pin
 *
 */
enum Detection : int
{
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

    /**
     * @brief open gpio device
     * 
     */
    void setup();

    /**
     * @brief Set the direction of a gpio pin
     * 
     * @param line_id BCM gpio id
     * @param direction Input or output
     */
    void set_direction(int line_id, Direction direction);
    
    /**
     * @brief Set the output high or low of a pin
     * 
     * @param line_id BCM gpio id
     * @param value High or low
     */
    void set_output(int line_id, Values value);

    /**
     * @brief Get the input value
     * 
     * @param line_id BCM gpio id
     * @return Values value of gpio pin
     */
    Values get_input(int line_id);

    /**
     * @brief Set the detection on pin input 
     * 
     * @param line_id BCM gpio id
     * @param detection detection direction (rising, falling, both)
     */
    void set_detection(int line_id, Detection detection);

    /**
     * @brief let thread wait for event on pin
     * 
     * @param line_id BCM gpio pin
     * @return true pin was triggered within timeout timeframe
     * @return false pin was not triggered within timeout timeframe
     */
    bool wait_for_event(int line_id);

    /**
     * @brief Set the timeout of trigger event
     * 
     * @param timeout maximum waiting time in nanoseconds;
     */
    void set_timeout(std::chrono::nanoseconds timeout);

    /**
     * @brief remove trigger from gpio line
     * 
     * @param line_id BCM gpio pin
     */
    void release_line(int line_id);
};

#endif