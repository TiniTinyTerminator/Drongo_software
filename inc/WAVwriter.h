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
    WAVWriter(uint8_t num_channels, uint32_t sample_rate, uint8_t bits_per_sample)
        : _num_channels(num_channels), _sample_rate(sample_rate), _bits_per_sample(bits_per_sample)
    {
        _bytes_to_write = std::ceil(bits_per_sample / 8);
    }

    ~WAVWriter()
    {
        close_file();
    }

    void open_file(const std::string &file_name)
    {
        if (_output_file.is_open())
        {
            close_file();
        }
        _output_file.open(file_name, std::ios::binary);
        if (!_output_file.is_open())
        {
            throw std::runtime_error("Could not open file for writing: " + file_name);
        }
        write_wav_header();
        write_info_chunk();
    }

    void close_file()
    {
        if (_output_file.is_open())
        {
            finalize_wav_header();
            _output_file.close();
        }
    }

    void write_channels(const std::vector<int32_t> &samples)
    {
        if (!_output_file.is_open())
        {
            throw std::runtime_error("File is not open for writing samples.");
        }

        if (samples.size() != _num_channels)
            throw std::runtime_error("Incorrect number of channels");

        for (const auto &sample : samples)
        {
            _output_file.write(reinterpret_cast<const char *>(&sample), _bytes_to_write);
        }
    }

    void write_samples(const std::vector<int32_t> &samples)
    {
        if (!_output_file.is_open())
        {
            throw std::runtime_error("File is not open for writing samples.");
        }

        for (const auto &sample : samples)
        {
            _output_file.write(reinterpret_cast<const char *>(&sample), _bytes_to_write);
        }
    }

private:
    std::ofstream _output_file;
    uint16_t _num_channels;
    uint32_t _sample_rate;
    uint16_t _bits_per_sample;
    uint16_t _bytes_to_write;
    std::streampos data_chunk_pos;

    void write_wav_header()
    {
        // Chunk descriptor
        const std::string chunk_id = "RIFF";
        const std::string format = "WAVE";

        // Subchunk1 (fmt subchunk)
        const std::string subchunk1_id = "fmt ";
        const int32_t subchunk1_size = 16; // For PCM
        const int16_t audio_format = 1;    // PCM
        const int32_t byte_rate = _sample_rate * _num_channels * _bits_per_sample / 8;
        const int16_t block_align = _num_channels * _bits_per_sample / 8;

        // Subchunk2 (data subchunk)
        const std::string subchunk2_id = "data";
        int32_t subchunk2_size = 0; // Placeholder for now, update after writing data

        // Write the header information in the correct order
        _output_file << chunk_id;
        _output_file.write(reinterpret_cast<const char *>(&subchunk2_size), sizeof(subchunk2_size));
        _output_file << format;
        _output_file << subchunk1_id;
        _output_file.write(reinterpret_cast<const char *>(&subchunk1_size), sizeof(subchunk1_size));
        _output_file.write(reinterpret_cast<const char *>(&audio_format), sizeof(audio_format));
        _output_file.write(reinterpret_cast<const char *>(&_num_channels), sizeof(_num_channels));
        _output_file.write(reinterpret_cast<const char *>(&_sample_rate), sizeof(_sample_rate));
        _output_file.write(reinterpret_cast<const char *>(&byte_rate), sizeof(byte_rate));
        _output_file.write(reinterpret_cast<const char *>(&block_align), sizeof(block_align));
        _output_file.write(reinterpret_cast<const char *>(&_bits_per_sample), sizeof(_bits_per_sample));
        _output_file << subchunk2_id;
        _output_file.write(reinterpret_cast<const char *>(&subchunk2_size), sizeof(subchunk2_size));
    }

    void write_info_chunk()
    {
        // Create an INFO chunk with creation date and time
        std::string creation_date = get_current_date_time();

        // INFO chunk
        _output_file << "LIST";
        int32_t list_chunk_size = 4 + 4 + creation_date.size() + 1; // 'ICRD', size, date string, null terminator
        _output_file << list_chunk_size;
        _output_file << "INFO";
        _output_file << "ICRD";
        int32_t date_chunk_size = static_cast<int32_t>(creation_date.size() + 1); // Include null terminator
        _output_file << date_chunk_size;
        _output_file << creation_date << '\0';
    }

    void finalize_wav_header()
    {
        if (!_output_file.is_open())
        {
            throw std::runtime_error("File is not open for finalizing WAV header.");
        }

        _output_file.seekp(data_chunk_pos, std::ios::beg);

        std::streampos file_size = _output_file.tellp();

        // RIFF chunk size
        int32_t chunk_size = static_cast<int32_t>(file_size) - 8;
        _output_file.seekp(4, std::ios::beg);
        _output_file.write(reinterpret_cast<char *>(&chunk_size), sizeof(chunk_size));

        // Format
        int16_t audio_format = 1; // PCM
        _output_file.seekp(20, std::ios::beg);
        _output_file.write(reinterpret_cast<char *>(&audio_format), sizeof(audio_format));

        // NumChannels
        _output_file.seekp(22, std::ios::beg);
        _output_file.write(reinterpret_cast<char *>(&_num_channels), sizeof(_num_channels));

        // SampleRate
        _output_file.seekp(24, std::ios::beg);
        _output_file.write(reinterpret_cast<char *>(&_sample_rate), sizeof(_sample_rate));

        // ByteRate
        int32_t byte_rate = _sample_rate * _num_channels * _bits_per_sample / 8;
        _output_file.seekp(28, std::ios::beg);
        _output_file.write(reinterpret_cast<char *>(&byte_rate), sizeof(byte_rate));

        // BlockAlign
        int16_t block_align = _num_channels * _bits_per_sample / 8;
        _output_file.seekp(32, std::ios::beg);
        _output_file.write(reinterpret_cast<char *>(&block_align), sizeof(block_align));

        // BitsPerSample
        _output_file.seekp(34, std::ios::beg);
        _output_file.write(reinterpret_cast<char *>(&_bits_per_sample), sizeof(_bits_per_sample));

        // Subchunk2Size (data size)
        int32_t subchunk2_size = static_cast<int32_t>(file_size) - 44;
        _output_file.seekp(40, std::ios::beg);
        _output_file.write(reinterpret_cast<char *>(&subchunk2_size), sizeof(subchunk2_size));
    }

    std::string get_current_date_time()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};
