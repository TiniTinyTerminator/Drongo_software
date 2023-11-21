#include "WAVwriter.h"

// Main chunk descriptor
const std::string _main_chunk_id = "RIFF";
const std::string _format = "WAVE";

// Subchunk1 (fmt subchunk)
const std::string _subchunk1_id = "fmt ";

constexpr int32_t _subchunk1_size = 16; // For PCM data

// constant (may be added to configuration in later versions)
constexpr int16_t _audio_format = 1;    // PCM

// Subchunk2 (data subchunk)
const std::string subchunk2_id = "data";

template <class T>
void write_as_bytes(std::ostream &_output_file, const T &obj)
{
    const char *ptr = reinterpret_cast<const char *>(&obj);

    // Write obj to stream
    _output_file.write(ptr, sizeof(obj));
}

WAVWriter::WAVWriter()
{
}

WAVWriter::WAVWriter(uint8_t num_channels, uint32_t sample_rate, uint8_t bits_per_sample)
    : _n_channels(num_channels),
      _sample_rate(sample_rate),
      _bits_per_sample(bits_per_sample),
      _bytes_to_write(bits_per_sample >> 3),
      _block_align((_n_channels * _bits_per_sample) >> 3),
      _byte_rate(_sample_rate * _block_align)
{
}

WAVWriter::~WAVWriter()
{
    close_file();
}

void WAVWriter::set_comments(const std::string &comment)
{
    _comments = comment;
}

void WAVWriter::set_datetime(const std::chrono::system_clock::time_point &datetime)
{
    _datetime = datetime;
}

void WAVWriter::open_file(const std::string &file_name)
{
    if (_output_file.is_open())
        close_file();

    _output_file.open(file_name, std::ios::binary);

    if (!_output_file.is_open())
        throw std::runtime_error("Could not open file for writing: " + file_name);

    write_wav_header();
}

void WAVWriter::close_file()
{
    if (_output_file.is_open())
    {
        write_info_chunk();
        finalize_wav_header();

        _output_file.close();
    }
}

void WAVWriter::set_n_channels(uint16_t num_channels)
{
    if(_output_file.is_open())
        throw std::runtime_error("cannot update number of channels while file is open");

    _n_channels = num_channels;

    _block_align = (_n_channels * _bits_per_sample) >> 3;
    _byte_rate = _sample_rate * _block_align;
}

void WAVWriter::set_sample_rate(uint32_t sample_rate)
{
    if(_output_file.is_open())
        throw std::runtime_error("cannot update sample rate while file is open");

    _sample_rate = sample_rate;

    _block_align = (_n_channels * _bits_per_sample) >> 3;
    _byte_rate = _sample_rate * _block_align;
}

void WAVWriter::set_bits_per_sample(uint16_t bits_per_sample)
{
    if(_output_file.is_open())
        throw std::runtime_error("cannot update bits per sample while file is open");

    _bits_per_sample = bits_per_sample; 
    _bytes_to_write = bits_per_sample >> 3;

    _block_align = (_n_channels * _bits_per_sample) >> 3;
    _byte_rate = _sample_rate * _block_align;
}



void WAVWriter::write_channels(const std::vector<int32_t> &samples)
{
    if (!_output_file.is_open())
        throw std::runtime_error("File is not open for writing samples.");

    if (samples.size() != _n_channels)
        throw std::runtime_error("Incorrect number of channels");

    _output_file.seekp(_current_data_chunk_pos);

    for (const auto &sample : samples)
        _output_file.write(reinterpret_cast<const char *>(&sample), _bytes_to_write);

    _current_data_chunk_pos = _output_file.tellp();
}

void WAVWriter::write_samples(const std::vector<int32_t> &samples)
{
    if (!_output_file.is_open())
        throw std::runtime_error("File is not open for writing samples.");

    _output_file.seekp(_current_data_chunk_pos);

    for (const auto &sample : samples)
        _output_file.write(reinterpret_cast<const char *>(&sample), _bytes_to_write);

    _current_data_chunk_pos = _output_file.tellp();
}

void WAVWriter::write_wav_header()
{
    constexpr int32_t _placeholder = 0xdeadbeef; // Placeholder for when still determining size

    // Write the header information in the correct order
    _output_file << _main_chunk_id;
    write_as_bytes(_output_file, _placeholder);
    _output_file << _format;
    _output_file << _subchunk1_id;
    write_as_bytes(_output_file, _subchunk1_size);
    write_as_bytes(_output_file, _audio_format);
    write_as_bytes(_output_file, _n_channels);
    write_as_bytes(_output_file, _sample_rate);
    write_as_bytes(_output_file, _byte_rate);
    write_as_bytes(_output_file, _block_align);
    write_as_bytes(_output_file, _bits_per_sample);
    _output_file << subchunk2_id;
    write_as_bytes(_output_file, _placeholder);
    _current_data_chunk_pos = _output_file.tellp();
}

void WAVWriter::write_info_chunk()
{
    _output_file.seekp(0, std::ios::end);

    auto write_string_chunk = [&](const std::string &chunk_id, const std::string &str)
    {
        _output_file << chunk_id; // Write the chunk ID
        int32_t str_len = str.size() + ((str.size() % 2) ? 1 : 2);
        write_as_bytes(_output_file, static_cast<int32_t>(str_len)); // Write the size as little-endian
        _output_file << str;                                         // Write the string data
        if (str.size() % 2)
            write_as_bytes(_output_file, static_cast<int8_t>(0x0)); // Null terminator to end the string
        else
            write_as_bytes(_output_file, static_cast<int16_t>(0x0)); // 2 Null terminators to end the string
    };

    std::stringstream ss;
    time_t in_time_t = std::chrono::system_clock::to_time_t(_datetime);
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");

    // Placeholder for LIST chunk size, to be overwritten later
    _output_file << "LIST";
    std::streampos size_pos = _output_file.tellp();
    write_as_bytes(_output_file, static_cast<int32_t>(0xdeadbeef)); // Temporary size placeholder

    _output_file << "INFO"; // Write INFO

    // Write the ICRD and ICMT sub-chunks
    write_string_chunk("ICRD", ss.str());
    write_string_chunk("ICMT", _comments);

    // Go back and write the real size of the LIST chunk
    std::streampos end_pos = _output_file.tellp();

    _output_file.seekp(size_pos);
    write_as_bytes(_output_file, static_cast<int32_t>(end_pos - size_pos - sizeof(int32_t))); // Write the size at the placeholder position

    // Restore the position to the end of the stream
    _output_file.seekp(end_pos);
}

void WAVWriter::finalize_wav_header()
{
    if (!_output_file.is_open())
        throw std::runtime_error("File is not open for finalizing WAV header.");

    _output_file.seekp(0, std::ios::end);

    std::streampos file_size = _output_file.tellp();

    // RIFF chunk size
    int32_t chunk_size = static_cast<int32_t>(file_size) - 8;
    _output_file.seekp(4, std::ios::beg);
    write_as_bytes(_output_file, chunk_size);

    // Subchunk2Size (data size)
    int32_t subchunk2_size = static_cast<int32_t>(_current_data_chunk_pos) - 44;
    _output_file.seekp(40, std::ios::beg);
    write_as_bytes(_output_file, subchunk2_size);
}