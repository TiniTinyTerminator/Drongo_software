/**
 * @file FftWindow.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-11-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <exception>
#include <iostream>
#include <iterator>
#include <cmath>


#include "FftWindow.h"


FftWindow::FftWindow(const size_t len) : _length(len), _half_fft_length((_length >> 1) + 1)
{
    update_plan();
}

void FftWindow::update_plan()
{
    _input = static_cast<double *>(fftw_malloc(sizeof(double) * _length));
    _output = static_cast<fftw_complex *>(fftw_malloc(sizeof(fftw_complex) * _half_fft_length));

    if (_input == nullptr)
        throw std::runtime_error("could not allocate memory for input buffer");
    if (_output == nullptr)
        throw std::runtime_error("could not allocate memory for output buffer");

    _plan = fftw_plan_dft_r2c_1d(_length, _input, _output, FFTW_ESTIMATE);
}

void FftWindow::add_data_point(double data)
{
    _buffer.push_back(data);

    while (_buffer.size() > _length)
        _buffer.pop_front();
}

void FftWindow::clear_data()
{
    _buffer.clear();
}

void FftWindow::calculate_fft()
{
    std::copy(_buffer.begin(), _buffer.begin() + _length, _input);

    fftw_execute(_plan);
}

std::vector<double> FftWindow::determine_absolutes()
{
    std::vector<double> result(_half_fft_length);

    for (int32_t i = 0; i < _half_fft_length; i++)
    {
        auto [real, imag] = _output[i];

        result[i] = std::sqrt(real * real + imag * imag);
    }

    return result;
}

FftWindow::~FftWindow()
{
    fftw_destroy_plan(_plan);

    fftw_free(_input);
    fftw_free(_output);

    fftw_cleanup();
}
