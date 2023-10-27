#include <iostream>
#include <chrono>
#include <thread>

#include "gpio.h"

using namespace std::chrono_literals;

int main(int argc, char *argv[])
{

    std::cout << "hello world!" << std::endl;

    Gpio gpio("/dev/gpiochip0");

    gpio.set_direction(16, Direction::OUTPUT);

    while (true)
    {
        gpio.set_output(16, Values::HIGH);
        std::cout << "high" << std::endl;
        std::this_thread::sleep_for(500ms);

        gpio.set_output(16, Values::LOW);
        std::cout << "low" << std::endl;
        std::this_thread::sleep_for(500ms);
    }
    
    return 0;
}