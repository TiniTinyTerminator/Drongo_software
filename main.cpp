#include <iostream>
#include <chrono>
#include <thread>
#include <bitset>
#include <typeinfo>

#include "gpio.h"
#include "spi.h"

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

    Spi spi("/dev/spidev0.0");

    spi.set_speed(10e6);

    LOG(INFO) << spi.get_speed();

    SpiModeConfig config = MODE_1;


    spi.set_mode(config);

    LOG(INFO) << std::bitset<16>(spi.get_mode().data);

    return 0;
}