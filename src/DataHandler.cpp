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

        std::this_thread::sleep_for(200ms);

        _adc.pwdn(false);

        std::this_thread::sleep_for(200ms);

        _adc.reset(false);

        std::this_thread::sleep_for(200ms);

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

        LOG_IF(tries > 0, WARNING) << "tried setting up adc " << tries << " times";
    } while (tries++ < max_tries);

    if (tries >= max_tries)
        throw std::runtime_error("could not setup adc");

    _active_channels = _adc.get_active_channels();
    _n_active_channels = _active_channels.size();
    
    _sample_rate = channel_drate_delay_to_frequency(_n_active_channels, AUTO_DRATE1, DLY0);
    
    _n_samples_per_file = _sample_rate * 30;

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
    _run_irq_thread = true;

    _irq_thread = std::thread(&DataHandler::irq_thread_func, this);
}

void DataHandler::irq_thread_stop(void)
{
    _run_irq_thread = false;

    _irq_thread.join();
}

void DataHandler::storing_thread_start(void)
{
    _run_storing_thread = true;

    _storing_thread = std::thread(&DataHandler::storing_thread_func, this);
}

void DataHandler::storing_thread_stop(void)
{
    _run_storing_thread = false;

    _cv.notify_one();

    _storing_thread.join();
}

void DataHandler::irq_thread_func(void)
{
    set_realtime_priority();
    set_thread_affinity(0);

    std::chrono::time_point t_prev = std::chrono::system_clock::now(), t_curr = t_prev;

    int64_t t_sample_period = 1e9 / _sample_rate;

    const std::chrono::nanoseconds t_request_data_period(static_cast<long long>((long long)1e9 / AUTO_DRATE1 / 5));

    LOG(INFO) << "starting sampling";

    _adc.start(true);

    std::this_thread::sleep_for(1us);

    ChannelData previous;

    uint32_t i = 0;

    while (_run_irq_thread)
    {

        // LOG_EVERY_N(1000, INFO) << "still sampling";

        std::this_thread::sleep_for(20us);

        ChannelData current = _adc.get_data_read();

        if (previous == current)
            continue;

        if (std::find(_active_channels.begin(), _active_channels.end(), current.first) == _active_channels.end())
        {
            i++;
            LOG(WARNING) << "misread of adc data " << i << " times";

            if (i > 100)
            {
                LOG(WARNING) << "too many failures, restarting ADC";

                storing_thread_stop();

                setup_adc();

                std::this_thread::sleep_for(10ms);

                storing_thread_start();

                i = 0;
            }

            continue;
        }

        previous = current;

        std::unique_lock lock(_mailbox_mtx);

        _raw_data_queue.push_back(current);

        if (_raw_data_queue.size() >= 100)
        {
            _cv.notify_one();
        }
    }
}

void DataHandler::storing_thread_func(void)
{
    LOG(INFO) << "processing thread starting";

    _current_timestamp = std::chrono::system_clock::now();

    uint32_t sample_counter = 0;

    std::vector<Iir::ChebyshevII::LowPass<20>> filters(_n_active_channels);

    for (auto &filter : filters)
    {
        filter.setup(_sample_rate, 550, 40);
        filter.reset();
    }

    new_file();

    _writer.set_comments("GEO5X,GEO5Y,GEO5Z,GEO4X,GEO4Y,GEO4Z,GEO3X,GEO3Y,GEO3Z,GEO2X,GEO2Y,GEO2Z");

    std::deque<std::vector<int32_t>> sorted_sample_queue;

    while (_run_storing_thread)
    {

        int32_t i = 0, c = 0;

        while (_raw_data_queue.size() >= _n_active_channels && _run_storing_thread && sorted_sample_queue.size() < 200)
        {
            std::vector<int32_t> samples(_n_active_channels);

            for (uint32_t c = 0; c < _n_active_channels; c++)
            {

                std::unique_lock lock(_mailbox_mtx);

                _cv.wait(lock, [&]()
                         { return _raw_data_queue.size() >= 100 || !_run_storing_thread; });

                if (!_run_storing_thread)
                    break;

                ChannelData channel_sample = _raw_data_queue.front();

                _raw_data_queue.pop_front();

                if (channel_sample.first != _active_channels[i])
                {
                    std::vector<uint8_t>::iterator data_point = std::find(_active_channels.begin(), _active_channels.end(), channel_sample.first);

                    if (data_point == _active_channels.end())
                        continue;

                    i = std::distance(_active_channels.begin(), data_point);
                }

                samples[i] = channel_sample.second;

                i = i < _n_active_channels - 1 ? i + 1 : 0;
            }

            sorted_sample_queue.push_back(samples);
        }

        std::deque<std::vector<int32_t>> previous_samples;

        while (sorted_sample_queue.size() > 10)
        {
            std::vector<int32_t> sample = sorted_sample_queue.front();

            for (uint32_t i = 0; i < _n_active_channels; i++)
            {
                if (sample[i] == 0)
                    for (auto next = sorted_sample_queue.begin() + 1; next != sorted_sample_queue.end(); next++)
                    {
                        if (next->at(i) != 0)
                        {
                            sample[i] = next->at(i);
                            break;
                        }
                    }

                sample[i] = filters[i].filter(sample[i]);
            }

            // for (uint32_t i = 0; i < _n_active_channels; i++)
            // {
            //     if (sample[i] == 0)
            //     {
            //         auto prev_iter = previous_samples.rbegin();        // Reverse iterator for previous_samples
            //         auto next_iter = sorted_sample_queue.begin() + 1; // Iterator for sorted_sample_queue

            //         float prev_value = 0;
            //         float next_value = 0;
            //         bool prev_found = false, next_found = false;

            //         // Search for the previous non-zero value using reverse iterator
            //         for (; prev_iter != previous_samples.rend(); ++prev_iter)
            //         {
            //             if (prev_iter->at(i) != 0)
            //             {
            //                 prev_value = prev_iter->at(i);
            //                 prev_found = true;
            //                 break;
            //             }
            //         }

            //         // Search for the next non-zero value
            //         for (; next_iter != sorted_sample_queue.end(); ++next_iter)
            //         {
            //             if (next_iter->at(i) != 0)
            //             {
            //                 next_value = next_iter->at(i);
            //                 next_found = true;
            //                 break;
            //             }
            //         }

            //         if (prev_found && next_found)
            //         {
            //             // Calculate the distance between the found points and adjust the ratio accordingly
            //             // Assuming each step in the iterators corresponds to the same time increment
            //             int backwardDistance = std::distance(previous_samples.rbegin(), prev_iter);
            //             int forwardDistance = std::distance(sorted_sample_queue.begin(), next_iter) - 1;
            //             int totalDistance = backwardDistance + forwardDistance;
            //             float ratio = static_cast<float>(backwardDistance) / totalDistance;
            //             sample[i] = prev_value + ratio * (next_value - prev_value);
            //         }
            //         else
            //         {
            //             LOG(WARNING) << "no substitute found for lost sample, copying next";

            //             for (auto next = sorted_sample_queue.begin() + 1; next != sorted_sample_queue.end(); next++)
            //             {
            //                 if (next->at(i) != 0)
            //                 {
            //                     sample[i] = next->at(i);
            //                     break;
            //                 }
            //             }
            //         }
                // }

                // sample[i] = filters[i].filter(sample[i]);
            // }

            _writer.write_channels(sample);
            
            sample_counter++;

            previous_samples.push_front(sorted_sample_queue.front());

            while (previous_samples.size() > 100)
                previous_samples.pop_back();

            sorted_sample_queue.pop_front();
        }

        if (sample_counter > _n_samples_per_file)
        {
            _current_timestamp = std::chrono::system_clock::now();

            sample_counter = 0;

            new_file();
        }

    }

    _writer.close_file();

    LOG(INFO) << "processing thread stopped";
}