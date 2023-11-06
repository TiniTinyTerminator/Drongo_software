#include <iostream>
#include <chrono>
#include <thread>
#include <string>

#include "WAVwriter.h"
#include "Ads1258.h"


#include "easylogging++.h"

using namespace std::chrono_literals;

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
    START_EASYLOGGINGPP(argc, argv);

    el::Configurations logging_conf;

    logging_conf.set(el::Level::Global, el::ConfigurationType::Format, "%datetime{%A %d/%M/%Y %H:%m:%s} (%level): %msg");

    el::Loggers::reconfigureAllLoggers(logging_conf);

    LOG(INFO) << "hello world!";

    Ads1258 adc("/dev/spidev0.0", "/dev/gpiochip0");

    uint8_t tries = 5;

    while (tries != 0)
    {
        adc.pwdn(true);
        adc.reset(true);

        std::this_thread::sleep_for(50ms);

        adc.pwdn(false);

        std::this_thread::sleep_for(50ms);

        adc.reset(false);

        std::this_thread::sleep_for(50ms);

        adc.enable_sleep_mode(false);
        adc.enable_bypass(true);
        adc.enable_status(true);
        adc.enable_external_clock(false);
        adc.set_drate(DrateConfig::DRATE_31250SPS);
        adc.set_delay(DelayConfig::DLY_64us);
        adc.enable_auto(true);
        adc.set_auto_single_channel({.raw_data = 0b1110000000000000});

        if (adc.verify_settings())
            break;

        LOG(WARNING) << "tried setting up ADC " << 6 - tries << " times";
    }

    if (tries == 0)
        throw std::runtime_error("could not setup ADC");


    WAVWriter writer(3, channel_drate_delay_to_frequency(3, AUTO_DRATE2, DLY4), 24);

    writer.open_file("dada.wav");

    auto tstart = std::chrono::high_resolution_clock::now();

    auto t = tstart;

    LOG(INFO) << "starting sampling";

    adc.start(true);

    while (true)
    {
        std::map<char, int32_t> adc_data;
        std::vector<int32_t> samples;

        for(uint32_t i = 0; i < 3; i++)
        {
            adc.await_data_ready(10ms);

            ChannelData data = adc.get_data();

            samples.push_back(data.second);
        }

        if(samples.size() != 3) continue;

        writer.write_channels(samples);

        auto tn = std::chrono::high_resolution_clock::now();

        if(tn - t > 10s) break;
    }

    writer.close_file();

    return 0;
}