#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <bitset>
#include <typeinfo>

#include "gpio.h"
#include "spi.h"
#include "Ads1258.h"
#include "RaspberryPiGPIO.h"

#include "easylogging++.h"

using namespace std::chrono_literals;

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
    START_EASYLOGGINGPP(argc, argv);

    el::Configurations logging_conf;

    logging_conf.set(el::Level::Info, el::ConfigurationType::Format, "%datetime{%A %d/%M/%Y %H:%m:%s} (%level): %msg");

    el::Loggers::reconfigureAllLoggers(logging_conf);

    LOG(INFO) << "hello world!";

    Ads1258 adc("/dev/spidev0.0", "/dev/gpiochip0");

    // Spi spi("/dev/spidev0.0");

    // spi.set_speed(31200000);

    std::this_thread::sleep_for(50ms);

    adc.pwdn(false);
    adc.reset(false);
    adc.start(true);

    std::this_thread::sleep_for(50ms);

    adc.enable_auto(true);

    adc.enable_sleep_mode(false);

    adc.enable_status(false);

    adc.enable_auto(true);
    adc.set_drate(DrateConfig::DRATE_7813SPS);
    adc.set_delay(DelayConfig::DLY_0us);
    adc.set_auto_single_channel({.channels = {.channel0 = true, .channel1 = true}});


    std::string str = std::string("hello, my name is max");

    while (true)
    {

        auto data = adc.get_data(); 
        
    }
    

    return 0;
}