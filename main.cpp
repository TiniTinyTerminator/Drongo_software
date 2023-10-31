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

    std::this_thread::sleep_for(50ms);

    adc.enable_sleep_mode(false);
    adc.enable_bypass(true);
    adc.enable_status(true);
    adc.enable_external_clock(false);
    adc.set_drate(DrateConfig::DRATE_1953SPS);
    adc.set_delay(DelayConfig::DLY_384us);
    adc.enable_auto(true);

    adc.start(true);

    adc.set_auto_single_channel({.channels = {.channel0 = true, .channel1 = true, .channel2 = true, .channel5 = true , .channel13 = true}});

    auto tn = std::chrono::high_resolution_clock::now();

    while (true)
    {
        adc.await_data_ready(10ms);

        std::vector<uint32_t> data = adc.get_data();

    //     for(auto a : data)
    //     {
    //         std::cout << std::bitset<32>(a) << '\t';
    //     }

    //     std::cout << std::endl;
    }
    

    return 0;
}