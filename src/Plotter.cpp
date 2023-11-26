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
#include <array>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <easylogging++.h>

extern "C"
{
#include <arm_neon.h>
}

#include "Plotter.h"

std::array<sf::Color, 12> colors = {
    sf::Color(255, 0, 0),
    sf::Color(0, 255, 0),
    sf::Color(0, 0, 255),
    sf::Color(255, 255, 0),
    sf::Color(255, 0, 255),
    sf::Color(0, 255, 255),
    sf::Color(255, 165, 0),
    sf::Color(128, 0, 128),
    sf::Color(191, 255, 0),
    sf::Color(255, 192, 203),
    sf::Color(0, 128, 128),
    sf::Color(255, 105, 180)
};



// void setup_fft_plant(void);
void Plotter::set_ranges(double t_span, double v_peak, double v_scale)
{
    _t_span = t_span;
    _v_peak = v_peak;
    _v_scale = v_scale;
}

void Plotter::set_bin_size(uint32_t size)
{
    _size = size;

    _plot_data = std::vector<std::vector<double>>(_size);
}

void Plotter::close(void)
{
    _window.close();
}

void Plotter::start(void)
{
    _window.setTitle("Signal window");
    _window.setSize(sf::Vector2u(1200, 800));
    _window.setFramerateLimit(24);

    _w = _window.getSize().x;
    _h = _window.getSize().y;

}

void Plotter::plot(void)
{
    std::vector<std::vector<sf::Vertex>> lines = std::vector<std::vector<sf::Vertex>>(_size, std::vector<sf::Vertex>(_plot_data.front().size()));

    for(uint32_t i = 0; i < _size; i++)
    {
        for(uint32_t j = 0; j < _plot_data.front().size(); j++)
        {
            lines[i][j] = sf::Vertex(sf::Vector2f(i, (_plot_data[i][j] *  _v_scale) - (_h >> 1)), colors[j]);
        }
    }

    for(auto line : lines)
    {
        _window.draw(line.data(), line.size(), sf::Lines);
    }
}

void Plotter::get_data(const std::deque<std::vector<double>> &data)
{
    if(data.begin() + _size > data.end())
        throw std::runtime_error("not enough data");

    std::copy(data.begin(), data.begin() + _size, _plot_data.begin());

}