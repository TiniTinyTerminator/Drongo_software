/**
 * @file IirFilter.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-11-15
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <vector>
#include <stdexcept>

#include <array>
#include <vector>

class BiquadFilter {
private:
    double b0, b1, b2, a0, a1, a2;
    std::array<double, 2> x_buffer = {0.0, 0.0};
    std::array<double, 2> y_buffer = {0.0, 0.0};

public:
    BiquadFilter(double b0, double b1, double b2, double a0, double a1, double a2)
        : b0(b0), b1(b1), b2(b2), a0(a0), a1(a1), a2(a2) {}

    double process(double input) {
        double output = (b0/a0) * input + (b1/a0) * x_buffer[0] + (b2/a0) * x_buffer[1]
                        - (a1/a0) * y_buffer[0] - (a2/a0) * y_buffer[1];

        x_buffer[1] = x_buffer[0];
        x_buffer[0] = input;

        y_buffer[1] = y_buffer[0];
        y_buffer[0] = output;

        return output;
    }
};


class NthOrderFilter {
private:
    std::vector<BiquadFilter> biquadFilters;

public:
    // Expecting two vectors of vectors: one for b coefficients and one for a coefficients
    NthOrderFilter(const std::vector<std::vector<double>>& bCoefficients, 
                   const std::vector<std::vector<double>>& aCoefficients) {
        if (bCoefficients.size() != aCoefficients.size()) {
            throw std::invalid_argument("The number of b and a coefficient sets must be equal.");
        }

        for (size_t i = 0; i < bCoefficients.size(); ++i) {
            const auto& bCoeffs = bCoefficients[i];
            const auto& aCoeffs = aCoefficients[i];

            if (bCoeffs.size() != 3 || aCoeffs.size() != 3) {
                throw std::invalid_argument("Each biquad stage must have exactly 3 b coefficients and 3 a coefficients.");
            }

            biquadFilters.emplace_back(bCoeffs[0], bCoeffs[1], bCoeffs[2], aCoeffs[0], aCoeffs[1], aCoeffs[2]);
        }
    }

    double process(double input) {
        double output = input;
        for (auto& filter : biquadFilters) {
            output = filter.process(output);
        }
        return output;
    }
};
