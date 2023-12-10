/**
 * @file DataHandler.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-11-13
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <thread>
#include <mutex>
#include <condition_variable>

#include <deque>
#include <atomic>
#include <chrono>
#include <filesystem>

#include "Ads1258.h"
#include "WAVwriter.h"
// #include "Plotter.h"

class DataHandler
{
        
private:
    /* data */
    Ads1258 _adc;
    WAVWriter _writer;
    
    std::thread _irq_thread;
    std::thread _storing_thread; 

    std::atomic_bool _run_irq_thread = true;
    std::atomic_bool _run_storing_thread = true;

    std::deque<ChannelData> _raw_data_queue;

    std::mutex _mailbox_mtx;

    std::condition_variable _cv_raw_data;
    std::condition_variable _cv_fft;

    std::vector<uint8_t> _active_channels;
    uint8_t _current_channel;
    uint8_t _n_active_channels;

    std::string _current_filename;
    std::filesystem::path _data_path;

    double _sample_rate;
    uint32_t _n_samples_per_file;

    std::chrono::system_clock::time_point _current_timestamp;



public:
    void set_data_path(std::filesystem::path path);
    void setup_adc(const uint32_t n_channels, const uint32_t max_tries = 10);
    
    void new_file(void);
    void delete_last_file(void);

    void irq_thread_start(void);
    void irq_thread_stop(void);

    void storing_thread_start(void);
    void storing_thread_stop(void);

    void irq_thread_func(void);
    void storing_thread_func(void);

    DataHandler();
    ~DataHandler();
};

#endif