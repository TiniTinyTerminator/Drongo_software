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

#include <fftw3.h>



class FftWindow
{
private:
    size_t _length;
    size_t _width;

    fftw_plan _plan;

    fftw_complex *_input, *_output;


public:
    FftWindow() = default;
    ~FftWindow();
};

FftWindow::FftWindow(/* args */)
{
}

FftWindow::~FftWindow()
{
    fftw_destroy_plan(_plan);

    fftw_free(_input);
    fftw_free(_output);

    fftw_cleanup();
}
