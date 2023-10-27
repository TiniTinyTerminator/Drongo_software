#include <iostream>

#include "gpiod.h"

int main(int argc, char * argv[])
{

    std::cout << "hello world!" << std::endl;

    gpiod_chip *chip = gpiod_chip_open("/dev/chip");

    

    return 0;
}