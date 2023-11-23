/**
 * @file FftWindow.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-11-22
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef FFTWINDOW_H
#define FFTWINDOW_H

#include <deque>
#include <cinttypes>
#include <vector>

#include <fftw3.h>

class FftWindow
{
private:
    const size_t _length;
    const size_t _half_fft_length;

    std::deque<double> _buffer;

    fftw_plan _plan;

    double *_input;
    fftw_complex *_output;

    void update_plan();

public:
    FftWindow(const size_t len);
    ~FftWindow();

    void add_data_point(double data);
    void clear_data();

    void calculate_fft();
    std::vector<double> determine_absolutes();
};

#endif