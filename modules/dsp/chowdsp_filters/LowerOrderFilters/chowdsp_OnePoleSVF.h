#pragma once

#include <array>
#include <cmath>

namespace chowdsp
{
/** Filter type options for State Variable Filters */
enum class State_Variable_Filter_Type
{
    Lowpass,
    Bandpass,
    Highpass,
    Notch,
    Allpass,
    Bell,
    LowShelf,
    HighShelf,
};

template <typename Sample_Type, State_Variable_Filter_Type type, size_t num_channels>
struct OnePoleSVF
{
    void prepare (double sample_rate)
    {
        T = static_cast<Sample_Type> (1.0 / sample_rate);
    }

    void reset() noexcept
    {
        std::fill (ic1eq.begin(), ic1eq.end(), Sample_Type {});
    }

    void set_cutoff_frequency (Sample_Type cutoff_hz)
    {
        const auto w = (Sample_Type) M_PI * cutoff_hz * T;
        g = std::tan (w); // @TODO: approximate?
        g1_r = Sample_Type (1) / (Sample_Type (1) + g);
    }

    Sample_Type process_sample (int channel, Sample_Type x) noexcept
    {
        Sample_Type y;
        if (type == State_Variable_Filter_Type::Lowpass)
        {
            y = (g * x + ic1eq[channel]) * g1_r;
            ic1eq[channel] = Sample_Type (2) * y - ic1eq[channel];
        }
        else if (type == State_Variable_Filter_Type::Highpass)
        {
            y = (x - ic1eq[channel]) * g1_r;
            ic1eq[channel] = Sample_Type (2) * (x - y) - ic1eq[channel];
        }

        return y;
    }

    Sample_Type T {};
    Sample_Type g {};
    Sample_Type g1_r {};
    std::array<Sample_Type, num_channels> ic1eq {};
};
} // namespace chowdsp
