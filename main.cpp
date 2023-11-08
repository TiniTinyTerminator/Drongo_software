#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <queue>

#include "easylogging++.h"
#include "WAVwriter.h"

#include <pthread.h>
#include "LinuxScheduling.h"

#include "Ads1258.h"

using namespace std::chrono_literals;

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
    START_EASYLOGGINGPP(argc, argv);

    el::Configurations logging_conf;

    logging_conf.set(el::Level::Global, el::ConfigurationType::Format, "%datetime{%A %d/%M/%Y %H:%m:%s} (%level): %msg");

    el::Loggers::reconfigureAllLoggers(logging_conf);

    setRealTimePriority();
    setThreadAffinity(0);

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
        adc.enable_bypass(false);
        adc.enable_status(false);
        adc.enable_external_clock(false);
        adc.set_drate(DrateConfig::DRATE_125000SPS);
        adc.set_delay(DelayConfig::DLY_0us);
        adc.enable_auto(true);
        adc.set_auto_single_channel({.raw_data = 0b1110000000000000});

        if (adc.verify_settings())
            break;

        LOG(WARNING) << "tried setting up ADC " << 6 - tries << " times";
    }

    if (tries == 0)
        throw std::runtime_error("could not setup ADC");

    WAVWriter writer(3, channel_drate_delay_to_frequency(3, AUTO_DRATE0, DLY0), 24);

    uint32_t period_ns = 1.0f / channel_drate_delay_to_frequency(3, AUTO_DRATE0, DLY0) * 1.0e9f;

    auto tstart = std::chrono::system_clock::now();

    writer.open_file("dada.wav");

    writer.set_comments("GEO5X,GEO5Y,GEO5Z,GEO4X,GEO4Y,GEO4Z,GEO3X,GEO3Y,GEO3Z,GEO2X,GEO2Y,GEO2Z");
    writer.set_datetime(tstart);

    auto t = tstart;

    LOG(INFO) << "starting sampling";

    adc.start(true);

    while (true)
    {
        // std::vector<int32_t> samples;
        // std::stringstream ss;

        for (uint32_t i = 0; i < 3; i++)
        {
            adc.await_data_ready();
            std::this_thread::sleep_for(10us);
            ChannelData data = adc.get_data_read();
            std::this_thread::sleep_for(10us);
            ChannelData data2 = adc.get_data_read();
            std::this_thread::sleep_for(10us);
            ChannelData data3 = adc.get_data_read();
            std::this_thread::sleep_for(10us);
            // if (i == 0 && data.first != 21)
            // {
            //     throw std::runtime_error("missed");

            //     while (data.first != 21)
            //     {
            //         adc.await_data_ready(10ms);

            //         data = adc.get_data_direct();
            //     }
            // }

            // if(data != data2 || data3 != data2 || data != data3) throw std::runtime_error("wrong read: " + std::to_string(data.second) + " " + std::to_string(data2.second) + " " + std::to_string(data3.second) + " - " + std::to_string(data.first) + " " + std::to_string(data2.first) + " " + std::to_string(data3.first));

            // ss << (uint32_t)data.first << " ";

            // samples.push_back(data.second);
        }

        auto tn = std::chrono::system_clock::now();

        const double freq = 1.0e9 / (double)(tn - t).count();

        if (tn - tstart > 120s)
            break;

        t = tn;

        std::cerr << freq << " " << channel_drate_delay_to_frequency(3, AUTO_DRATE0, DLY0) << std::endl;

        // writer.write_samples(samples);


    }

    writer.close_file();

    return 0;
}