/**
 * @file DataHandler.h
 * @author Max Bensink (maxbensink@outlook.com)
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
    Ads1258 _adc; ///< Object for ADS1258 ADC interface.
    WAVWriter _writer; ///< Object for writing data to WAV files.
    
    std::thread _irq_thread; ///< Thread handling IRQ (Interrupt Requests).
    std::thread _storing_thread; ///< Thread for storing data into files.

    std::atomic_bool _run_irq_thread = true; ///< Control flag for the IRQ thread.
    std::atomic_bool _run_storing_thread = true; ///< Control flag for the storing thread.

    std::deque<ChannelData> _raw_data_queue; ///< Queue for raw data from ADC.

    std::mutex _mailbox_mtx; ///< Mutex for controlling access to the data queue.

    std::condition_variable _cv_raw_data; ///< Condition variable for new raw data.
    std::condition_variable _cv_fft; ///< Condition variable for FFT processing (if applicable).

    std::vector<uint8_t> _active_channels; ///< Active channels in the ADC.
    uint8_t _current_channel; ///< Current channel being processed.
    uint8_t _n_active_channels; ///< Number of active channels.

    std::string _current_filename; ///< Name of the current data file.
    std::filesystem::path _data_path; ///< Path for storing data files.

    double _sample_rate; ///< Sampling rate of the ADC.
    uint32_t _n_samples_per_file; ///< Number of samples per file.

    std::chrono::system_clock::time_point _current_timestamp; ///< Current timestamp for data samples.

public:
    DataHandler();
    ~DataHandler();

    /**
     * @brief Set the data path for storing data files.
     * 
     * @param path Filesystem path for data storage.
     */
    void set_data_path(std::filesystem::path path);

    /**
     * @brief Set up the ADC with the specified number of channels.
     * 
     * @param n_channels Number of channels to activate on the ADC.
     * @param max_tries Maximum attempts for setting up the ADC. Defaults to 10.
     */
    void setup_adc(const uint32_t n_channels, const uint32_t max_tries = 10);

    /**
     * @brief Create a new file for storing data.
     */
    void new_file(void);

    /**
     * @brief Delete the last created data file.
     */
    void delete_last_file(void);

    /**
     * @brief Start the interrupt request (IRQ) thread.
     */
    void irq_thread_start(void);

    /**
     * @brief Stop the IRQ thread.
     */
    void irq_thread_stop(void);

    /**
     * @brief Start the data storing thread.
     */
    void storing_thread_start(void);

    /**
     * @brief Stop the data storing thread.
     */
    void storing_thread_stop(void);

    /**
     * @brief Function executed by the IRQ thread.
     */
    void irq_thread_func(void);

    /**
     * @brief Function executed by the data storing thread.
     */
    void storing_thread_func(void);

};

#endif