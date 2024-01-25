/**
 * @file RaspberryPiGPIO.h
 * @author Max Bensink (maxbensink@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-10-30
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef RASPBERRY_PI_GPIO_H
#define RASPBERRY_PI_GPIO_H

namespace RaspberryPi
{

    // Define an enum  for the physical pin numbers to BCM mapping.
    enum PhysicalToBCM : int
    {
        // Mapping of physical pin numbers to BCM GPIO numbers.
        // This list is not exhaustive and is specific to 40-pin models.
        // Please refer to the specific Raspberry Pi model's GPIO pinout diagram.
        PIN3 = 2,   //<Physical pin 3 to BCM GPIO2 (I2C1 SDA)
        PIN5 = 3,   //<Physical pin 5 to BCM GPIO3 (I2C1 SCL)
        PIN7 = 4,   //<Physical pin 7 to BCM GPIO4
        PIN8 = 14,  //<Physical pin 8 to BCM GPIO14 (UART0 TX)
        PIN10 = 15, //<Physical pin 10 to BCM GPIO15 (UART0 RX)
        PIN11 = 17, //<Physical pin 11 to BCM GPIO17
        PIN12 = 18, //<Physical pin 12 to BCM GPIO18 (PCM CLK)
        PIN13 = 27, //<Physical pin 13 to BCM GPIO27
        PIN15 = 22, //<Physical pin 15 to BCM GPIO22
        PIN16 = 23, //<Physical pin 16 to BCM GPIO23
        PIN18 = 24, //<Physical pin 18 to BCM GPIO24
        PIN19 = 10, //<Physical pin 19 to BCM GPIO10 (SPI0 MOSI)
        PIN21 = 9,  //<Physical pin 21 to BCM GPIO9 (SPI0 MISO)
        PIN22 = 25, //<Physical pin 22 to BCM GPIO25
        PIN23 = 11, //<Physical pin 23 to BCM GPIO11 (SPI0 SCLK)
        PIN24 = 8,  //<Physical pin 24 to BCM GPIO8 (SPI0 CE0)
        PIN26 = 7,  //<Physical pin 26 to BCM GPIO7 (SPI0 CE1)
        PIN29 = 5,  //<Physical pin 29 to BCM GPIO5
        PIN31 = 6,  //<Physical pin 31 to BCM GPIO6
        PIN32 = 12, //<Physical pin 32 to BCM GPIO12
        PIN33 = 13, //<Physical pin 33 to BCM GPIO13 (PWM1)
        PIN35 = 19, //<Physical pin 35 to BCM GPIO19 (PCM FS)
        PIN36 = 16, //<Physical pin 36 to BCM GPIO16
        PIN37 = 26, //<Physical pin 37 to BCM GPIO26
        PIN38 = 20, //<Physical pin 38 to BCM GPIO20
        PIN40 = 21  //<Physical pin 40 to BCM GPIO21
    };

} // namespace RaspberryPi

#endif // RASPBERRY_PI_GPIO_H
