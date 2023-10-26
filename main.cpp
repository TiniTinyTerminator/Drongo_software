#include <iostream>

#include "gpio.h"

int main(int argc, char * argv[])
{

    std::cout << "hello world!" << std::endl;

    Gpio gpio("/dev/gpiochip0");

    gpio.set_direction(17, Direction::OUTPUT);

    return 0;
}