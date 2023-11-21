
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
    WAVWriter(uint8_t num_channels, uint32_t sample_rate, uint8_t bits_per_sample);
    WAVWriter();
    ~WAVWriter();

    void set_comments(const std::string &comment);
    void set_datetime(const std::chrono::system_clock::time_point &datetime);
    void open_file(const std::string &file_name);
    void close_file();

    void write_channels(const std::vector<int32_t> &samples);
    void write_samples(const std::vector<int32_t> &samples);

    void set_n_channels(uint16_t num_channels);
    void set_sample_rate(uint32_t sample_rate);
    void set_bits_per_sample(uint16_t bits_per_sample);

private:

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

    void write_wav_header();

    void write_info_chunk();

    void finalize_wav_header();
};

#endif