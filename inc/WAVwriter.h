/**
 * @file WAVwriter.h
 * @author Max Bensink (maxbensink@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2024-01-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#ifndef WAVWRITER_H
#define WAVWRITER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <chrono>
#include <ctime>
#include <iomanip>

class WAVWriter
{
public:
public:
    WAVWriter(uint8_t num_channels, uint32_t sample_rate, uint8_t bits_per_sample);
    WAVWriter();
    ~WAVWriter();

    /**
     * @brief Set comments or metadata for the WAV file.
     * 
     * @param comment The comment or metadata string to be added to the file.
     */
    void set_comments(const std::string &comment);

    /**
     * @brief Set the datetime for the WAV file metadata.
     * 
     * @param datetime The timestamp to be recorded in the file.
     */
    void set_datetime(const std::chrono::system_clock::time_point &datetime);

    /**
     * @brief Open a new file for writing WAV data.
     * 
     * @param file_name The name of the file to be created and written to.
     */
    void open_file(const std::string &file_name);

    /**
     * @brief Close the currently open WAV file.
     */
    void close_file();

    /**
     * @brief Write a vector of samples to the WAV file as channels.
     * 
     * @param samples The vector of samples to be written.
     */
    void write_channels(const std::vector<int32_t> &samples);

    /**
     * @brief Write a vector of individual samples to the WAV file.
     * 
     * @param samples The vector of samples to be written.
     */
    void write_samples(const std::vector<int32_t> &samples);

    /**
     * @brief Set the number of channels for the WAV file.
     * 
     * @param num_channels The number of audio channels.
     */
    void set_n_channels(uint16_t num_channels);

    /**
     * @brief Set the sample rate for the WAV file.
     * 
     * @param sample_rate The sample rate in Hz.
     */
    void set_sample_rate(uint32_t sample_rate);

    /**
     * @brief Set the number of bits per sample for the WAV file.
     * 
     * @param bits_per_sample The number of bits in each audio sample.
     */
    void set_bits_per_sample(uint16_t bits_per_sample);

private:
    // Private member variables with their comments

    /**
     * @brief Write the header of the WAV file.
     */
    void write_wav_header();

    /**
     * @brief Write the 'INFO' chunk to the WAV file for additional metadata.
     */
    void write_info_chunk();

    /**
     * @brief Finalize the WAV file by updating the header with the correct file size.
     */
    void finalize_wav_header();

    // Sample size and speed information
    uint16_t _n_channels = 0;
    uint32_t _sample_rate = 0;
    uint16_t _bits_per_sample = 0;
    uint16_t _bytes_to_write = 0;

    int16_t _block_align = 0;
    int32_t _byte_rate = 0;

    // Stream objects;
    std::ofstream _output_file;
    std::streampos _current_data_chunk_pos;

    // Metadata
    std::chrono::system_clock::time_point _datetime;
    std::string _comments;

};



#endif