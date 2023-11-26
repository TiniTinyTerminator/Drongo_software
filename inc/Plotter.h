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

#include <SFML/Graphics.hpp>

#include <fftw3.h>

class Plotter
{
private:
    uint32_t _size;
    uint32_t _w, _h;
    
    sf::RenderWindow _window;
    
    // std::vector<double> _time_points;
    std::vector<std::vector<double>> _plot_data;


    double _v_scale, _v_peak, _t_span;

public:
    Plotter() = default;

    ~Plotter();

    void setup_axes(void);
    // void setup_fft_plant(void);
    void set_ranges(double t_span, double v_peak, double v_scale);
    void set_bin_size(uint32_t size);
    void close(void);
    void start(void);
    void plot(void);
    void get_data(const std::deque<std::vector<double>> &data);
};

#endif