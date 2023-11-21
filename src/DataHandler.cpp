/**
 * @file DataHandler.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-11-13
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <thread>
#include <chrono>
#include <ranges>
#include <condition_variable>

#include "easylogging++.h"
#include "utils/linux_scheduling.h"

#include "Ads1258.h"

#include "DataHandler.h"

using namespace std::chrono_literals;

DataHandler::DataHandler() : _adc("/dev/spidev0.0", "/dev/gpiochip0")
{
}

DataHandler::~DataHandler()
{
    _writer.close_file();
}

void DataHandler::setup_adc(const uint32_t max_tries = 10)
{
    uint32_t tries = 0;

    do    
    {
        _adc.pwdn(true);
        _adc.reset(true);

        std::this_thread::sleep_for(50ms);

        _adc.pwdn(false);

        std::this_thread::sleep_for(50ms);

        _adc.reset(false);

        std::this_thread::sleep_for(50ms);

        _adc.enable_sleep_mode(false);
        _adc.enable_bypass(false);
        _adc.enable_status(true);
        _adc.enable_external_clock(false);
        _adc.set_drate(DrateConfig::DRATE_1);
        _adc.set_delay(DelayConfig::DLY_0us);
        _adc.enable_auto(true);
        _adc.set_auto_single_channel({.raw_data = 0b1110000000000000});

        if (_adc.verify_settings())
            break;

        LOG(WARNING) << "tried setting up adc " << tries << " times";
    }
    while (tries++ < max_tries);

    if (tries >= max_tries)
        throw std::runtime_error("could not setup adc");

    _active_channels = _adc.get_active_channels();
    _n_active_channels = _active_channels.size();
    _sample_rate = channel_drate_delay_to_frequency(_n_active_channels, AUTO_DRATE1, DLY0);

    _writer.set_n_channels(_n_active_channels);
    _writer.set_bits_per_sample(24);
    _writer.set_sample_rate(_sample_rate);
}

void DataHandler::new_file(void)
{
    _writer.close_file();

    std::stringstream ss;
    time_t in_time_t = std::chrono::system_clock::to_time_t(_current_timestamp);
    ss << std::put_time(std::localtime(&in_time_t), "date-%Y-%m-%d-time-%H-%M-%S.wav");

    _writer.open_file(ss.str());

    _writer.set_datetime(_current_timestamp);
}

void DataHandler::irq_thread_start(void)
{
    _irq_thread = std::thread(&DataHandler::irq_thread_func, this);
}

void DataHandler::irq_thread_stop(void)
{
    _run_irq_thread = false;

    _irq_thread.join();
}

void DataHandler::processing_thread_start(void)
{
    _storing_thread = std::thread(&DataHandler::storing_thread_func, this);
}

void DataHandler::processing_thread_stop(void)
{
    _run_storing_thread = false;

    _data_ready = true;
    _cv.notify_one();

    _storing_thread.join();
}

void DataHandler::irq_thread_func(void)
{
    set_realtime_priority();
    set_thread_affinity(0);

    std::chrono::time_point t_prev = std::chrono::system_clock::now(), t_curr = t_prev;
    auto t_period = (t_curr - t_prev).count();

    int64_t t_sample_period = 1e9 / _sample_rate;

    // const std::chrono::nanoseconds t_request_data_period(static_cast<long long>(1e9 / AUTO_DRATE3) / 4);

    const timespec wait_time = {
        .tv_sec = 0,
        .tv_nsec = static_cast<long long>(1e9 / AUTO_DRATE1 / 5) 
    };

    timespec remainder = {0};

    LOG(INFO) << "starting sampling";

    _adc.start(true);

    std::this_thread::sleep_for(5us);


    while (_run_irq_thread)
    {

        uint32_t i = 0;

        std::vector<int32_t> samples(_n_active_channels, 0);
        std::vector<int32_t> channels(_n_active_channels, 0);

        ChannelData current, previous = current;
        std::stringstream ss;

        for (size_t c = 0; c < _n_active_channels; c++)
        {

            while (_run_irq_thread)
            {
                try
                {
                    current = _adc.get_data_read();
                }
                catch (const std::exception &e)
                {
                    LOG(ERROR) << e.what();
                    continue;
                }

                if (current != previous)
                    break;

                // t_curr = std::chrono::system_clock::now();
                // std::this_thread::sleep_for(t_request_data_period);
                nanosleep(&wait_time, &remainder);
                // LOG_EVERY_N(1000, INFO) << (t_curr - t_prev).count() << " nanoseconds";

                // t_prev = t_curr;
            }

            previous = current;

            if (_active_channels[i] != current.first)
            {
                std::vector<uint8_t>::iterator index = std::find(_active_channels.begin(), _active_channels.end(), current.first);

                if (index == _active_channels.end()) {
                    LOG(ERROR) << "lost connection to adc, restarting..."; 

                    processing_thread_stop();

                    setup_adc();

                    processing_thread_start();
                }
                else
                    i = std::distance(_active_channels.begin(), index);
            }

            samples[i] = current.second;
            channels[i] = current.first;

            ss << (uint32_t)i << " ";

            i = (i < _n_active_channels - 1) ? i + 1 : 0;

        }


        for(auto ch : channels)
        {
            ss << (uint32_t)ch << ' ';
        }

        LOG_EVERY_N(1000, INFO) << ss.str();

        std::lock_guard lock(_mailbox_mtx);

        _data_mailbox.push_back(samples);

        if (_data_mailbox.size() >= 100 && _data_ready == false)
        {
            _data_ready = true;
            _cv.notify_one();
        }
    }
}


void DataHandler::storing_thread_func(void)
{

    // set_realtime_priority();
    // set_thread_affinity(1);

    _current_timestamp = std::chrono::system_clock::now();
    auto t = _current_timestamp;

    std::vector<Iir::ChebyshevII::LowPass<20>> filters(_n_active_channels);
    // std::vector<Iir::ChebyshevII::BandPass<20>> filters(_n_active_channels);

    for (auto &filter : filters)
    {
        filter.setup(_sample_rate, 550, 40);
        filter.reset();
    }

    new_file();

    _writer.set_comments("GEO5X,GEO5Y,GEO5Z,GEO4X,GEO4Y,GEO4Z,GEO3X,GEO3Y,GEO3Z,GEO2X,GEO2Y,GEO2Z");

    while (_run_storing_thread)
    {

        std::unique_lock lock(_mailbox_mtx);

        while (!_data_mailbox.empty())
        {

            std::vector<int32_t> &data_first = _data_mailbox[0];

            for (uint32_t i = 0; i < _n_active_channels; i++)
            {
                for (uint32_t j = 1; j < _data_mailbox.size() && data_first[i] == 0; j++)
                {
                    if (_data_mailbox[j][i] != 0)
                    {
                        data_first[i] = _data_mailbox[j][i];
                        break;
                    }
                }

                data_first[i] = filters[i].filter(data_first[i]);
            }

            _writer.write_channels(data_first);

            _data_mailbox.pop_front();
        }

        if (_data_mailbox.empty())
        {
            _data_ready = false;
            _cv.wait(lock, [&]()
                     { return _data_ready; });
        }

        if (t - _current_timestamp > 30s)
        {
            _current_timestamp = t;

            new_file();
        }

        t = std::chrono::system_clock::now();
    }

    _writer.close_file();
}