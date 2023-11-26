/**
 * @file DirectForm2Neon.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-11-24
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef DIRECTFORMIINEON_H
#define DIRECTFORMIINEON_H

#include "Iir.h"

extern "C"
{
#include <arm_neon.h>
}

namespace Iir
{
    class DirectFormIINeon
    {
    public:
        DirectFormIINeon()
        {
            reset();
        };

        void reset()
        {
            m_v = vdupq_n_f64(0.0f);
        }

        double filter(const double in, const Biquad &s) volatile
        {
            float64x2_t a = {s.m_a1, s.m_a2}, b = {s.m_b1, s.m_b2};

            const float64x2_t a_mul = a * m_v;
            const float64x2_t b_mul = b * m_v;

            m_v[1] = m_v[0];
            m_v[0] = in - (a_mul[0] + a_mul[1]);

            return s.m_b0 * m_v[0] + (b_mul[0] + b_mul[1]);
        }

    private:
        float64x2_t m_v;
    };

}
#endif
