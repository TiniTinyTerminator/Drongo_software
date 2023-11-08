#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <chrono>
#include <ctime>
#include <iomanip>

template <class T>
void write_as_bytes(std::ostream &_output_file, const T &obj)
{
    const char *ptr = reinterpret_cast<const char *>(&obj);

    // Write obj to stream
    _output_file.write(ptr, sizeof(obj));
}

class WAVWriter
{
public:
    WAVWriter(uint8_t num_channels, uint32_t sample_rate, uint8_t bits_per_sample);
    ~WAVWriter();

    void set_comments(const std::string &comment);
    void set_datetime(const std::chrono::system_clock::time_point &datetime);
    void open_file(const std::string &file_name);
    void close_file();

    void write_channels(const std::vector<int32_t> &samples);
    void write_samples(const std::vector<int32_t> &samples);

private:
    // Main chunk descriptor
    const std::string _main_chunk_id = "RIFF";
    const std::string _format = "WAVE";

    // Subchunk1 (fmt subchunk)
    const std::string _subchunk1_id = "fmt ";
    const int32_t _subchunk1_size = 16; // For PCM
    const int16_t _audio_format = 1;    // PCM

    // Sample size and speed information
    const uint16_t _n_channels;
    const uint32_t _sample_rate;
    const uint16_t _bits_per_sample;
    const uint16_t _bytes_to_write;

    const int16_t _block_align;
    const int32_t _byte_rate;

    // Subchunk2 (data subchunk)
    const std::string subchunk2_id = "data";

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
