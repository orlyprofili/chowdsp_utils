/*
  Cortex-M7F CMSIS-DSP backed replacement for JUCE FloatVectorOperations.
  Drop-in compatible with the original file you posted (same symbols, templates,
  and namespace layout), but tuned for Arm M7F without NEON/vDSP/SSE.

  Compile with:
    -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
  Defines:
    -DARM_MATH_CM7
*/

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

#include <chowdsp_core/JUCEHelpers/juce_ExtraDefinitions.h>
#include <chowdsp_core/JUCEHelpers/juce_FloatVectorOperations.h>

extern "C"
{
#include "arm_math.h" // CMSIS-DSP
}

namespace chowdsp_juce
{
namespace FloatVectorHelpers
{
    inline void zeromem (void* memory, size_t numBytes) noexcept { std::memset (memory, 0, numBytes); }

    // -------------------- Scalar fallbacks for double and uncommon ops --------------------
    template <typename Size>
    static inline void scalar_add_amount (float* dest, float amt, Size n) noexcept
    {
        for (Size i = 0; i < n; ++i)
            dest[i] += amt;
    }

    template <typename Size>
    static inline void scalar_add_src (float* dest, const float* src, Size n) noexcept
    {
        for (Size i = 0; i < n; ++i)
            dest[i] += src[i];
    }

    template <typename Size>
    static inline void scalar_sub (float* dest, const float* src, Size n) noexcept
    {
        for (Size i = 0; i < n; ++i)
            dest[i] -= src[i];
    }

    template <typename Size>
    static inline void scalar_muladd (float* dest, const float* src, float k, Size n) noexcept
    {
        for (Size i = 0; i < n; ++i)
            dest[i] += src[i] * k;
    }

    template <typename Size>
    static inline void scalar_mulsub (float* dest, const float* src, float k, Size n) noexcept
    {
        for (Size i = 0; i < n; ++i)
            dest[i] -= src[i] * k;
    }

    template <typename Size>
    static inline void scalar_mul2 (float* dest, const float* a, const float* b, Size n) noexcept
    {
        for (Size i = 0; i < n; ++i)
            dest[i] = a[i] * b[i];
    }

    template <typename Size>
    static inline void scalar_add2 (float* dest, const float* a, const float* b, Size n) noexcept
    {
        for (Size i = 0; i < n; ++i)
            dest[i] = a[i] + b[i];
    }

    template <typename Size>
    static inline void scalar_sub2 (float* dest, const float* a, const float* b, Size n) noexcept
    {
        for (Size i = 0; i < n; ++i)
            dest[i] = a[i] - b[i];
    }

    template <typename Size>
    static inline void scalar_min_elem (float* dest, const float* a, const float* b, Size n) noexcept
    {
        for (Size i = 0; i < n; ++i)
            dest[i] = (a[i] < b[i]) ? a[i] : b[i];
    }

    template <typename Size>
    static inline void scalar_max_elem (float* dest, const float* a, const float* b, Size n) noexcept
    {
        for (Size i = 0; i < n; ++i)
            dest[i] = (a[i] > b[i]) ? a[i] : b[i];
    }

    template <typename Size>
    static inline void scalar_min_cmp (float* dest, const float* a, float c, Size n) noexcept
    {
        for (Size i = 0; i < n; ++i)
            dest[i] = (a[i] < c) ? a[i] : c;
    }

    template <typename Size>
    static inline void scalar_max_cmp (float* dest, const float* a, float c, Size n) noexcept
    {
        for (Size i = 0; i < n; ++i)
            dest[i] = (a[i] > c) ? a[i] : c;
    }

    template <typename Size>
    static inline void scalar_clip (float* dest, const float* src, float lo, float hi, Size n) noexcept
    {
        for (Size i = 0; i < n; ++i)
        {
            float x = src[i];
            if (x < lo)
                x = lo;
            if (x > hi)
                x = hi;
            dest[i] = x;
        }
    }

    // -------------------- float paths backed by CMSIS-DSP where available --------------------
    template <typename Size>
    void clear (float* dest, Size num) noexcept
    {
        zeromem (dest, (size_t) num * sizeof (float));
    }

    template <typename Size>
    void clear (double* dest, Size num) noexcept
    {
        zeromem (dest, (size_t) num * sizeof (double));
    }

    template <typename Size>
    void fill (float* dest, float valueToFill, Size num) noexcept
    {
        arm_fill_f32 (valueToFill, dest, (uint32_t) num);
    }

    template <typename Size>
    void fill (double* dest, double valueToFill, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] = valueToFill;
    }

    template <typename Size>
    void copyWithMultiply (float* dest, const float* src, float multiplier, Size num) noexcept
    {
        arm_scale_f32 (src, multiplier, dest, (uint32_t) num);
    }

    template <typename Size>
    void copyWithMultiply (double* dest, const double* src, double multiplier, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] = src[i] * multiplier;
    }

    template <typename Size>
    void add (float* dest, float amount, Size num) noexcept
    {
        // in-place offset is supported
        arm_offset_f32 (dest, amount, dest, (uint32_t) num);
    }

    template <typename Size>
    void add (double* dest, double amount, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] += amount;
    }

    template <typename Size>
    void add (float* dest, const float* src, float amount, Size num) noexcept
    {
        arm_offset_f32 (src, amount, dest, (uint32_t) num);
    }

    template <typename Size>
    void add (double* dest, const double* src, double amount, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] = src[i] + amount;
    }

    template <typename Size>
    void add (float* dest, const float* src, Size num) noexcept
    {
        arm_add_f32 (dest, src, dest, (uint32_t) num);
    }

    template <typename Size>
    void add (double* dest, const double* src, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] += src[i];
    }

    template <typename Size>
    void add (float* dest, const float* src1, const float* src2, Size num) noexcept
    {
        arm_add_f32 (src1, src2, dest, (uint32_t) num);
    }

    template <typename Size>
    void add (double* dest, const double* src1, const double* src2, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] = src1[i] + src2[i];
    }

    template <typename Size>
    void subtract (float* dest, const float* src, Size num) noexcept
    {
        // dest -= src  => dest = dest + (-src)
        // Use arm_sub_f32(dest, src, dest)
        arm_sub_f32 (dest, src, dest, (uint32_t) num);
    }

    template <typename Size>
    void subtract (double* dest, const double* src, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] -= src[i];
    }

    template <typename Size>
    void subtract (float* dest, const float* src1, const float* src2, Size num) noexcept
    {
        arm_sub_f32 (src1, src2, dest, (uint32_t) num);
    }

    template <typename Size>
    void subtract (double* dest, const double* src1, const double* src2, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] = src1[i] - src2[i];
    }

    template <typename Size>
    void addWithMultiply (float* dest, const float* src, float multiplier, Size num) noexcept
    {
        // No CMSIS vsma. Do tight scalar FMA pattern.
        scalar_muladd (dest, src, multiplier, num);
    }

    template <typename Size>
    void addWithMultiply (double* dest, const double* src, double multiplier, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] += src[i] * multiplier;
    }

    template <typename Size>
    void addWithMultiply (float* dest, const float* src1, const float* src2, Size num) noexcept
    {
        // dest += src1 * src2
        for (Size i = 0; i < num; ++i)
            dest[i] += src1[i] * src2[i];
    }

    template <typename Size>
    void addWithMultiply (double* dest, const double* src1, const double* src2, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] += src1[i] * src2[i];
    }

    template <typename Size>
    void subtractWithMultiply (float* dest, const float* src, float multiplier, Size num) noexcept
    {
        scalar_mulsub (dest, src, multiplier, num);
    }

    template <typename Size>
    void subtractWithMultiply (double* dest, const double* src, double multiplier, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] -= src[i] * multiplier;
    }

    template <typename Size>
    void subtractWithMultiply (float* dest, const float* src1, const float* src2, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] -= src1[i] * src2[i];
    }

    template <typename Size>
    void subtractWithMultiply (double* dest, const double* src1, const double* src2, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] -= src1[i] * src2[i];
    }

    template <typename Size>
    void multiply (float* dest, const float* src, Size num) noexcept
    {
        arm_mult_f32 (dest, src, dest, (uint32_t) num);
    }

    template <typename Size>
    void multiply (double* dest, const double* src, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] *= src[i];
    }

    template <typename Size>
    void multiply (float* dest, const float* src1, const float* src2, Size num) noexcept
    {
        arm_mult_f32 (src1, src2, dest, (uint32_t) num);
    }

    template <typename Size>
    void multiply (double* dest, const double* src1, const double* src2, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] = src1[i] * src2[i];
    }

    template <typename Size>
    void multiply (float* dest, float multiplier, Size num) noexcept
    {
        arm_scale_f32 (dest, multiplier, dest, (uint32_t) num);
    }

    template <typename Size>
    void multiply (double* dest, double multiplier, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] *= multiplier;
    }

    template <typename Size>
    void multiply (float* dest, const float* src, float multiplier, Size num) noexcept
    {
        arm_scale_f32 (src, multiplier, dest, (uint32_t) num);
    }

    template <typename Size>
    void multiply (double* dest, const double* src, double multiplier, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] = src[i] * multiplier;
    }

    template <typename Size>
    void negate (float* dest, const float* src, Size num) noexcept
    {
        arm_scale_f32 (src, -1.0f, dest, (uint32_t) num);
    }

    template <typename Size>
    void negate (double* dest, const double* src, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] = -src[i];
    }

    template <typename Size>
    void abs (float* dest, const float* src, Size num) noexcept
    {
        arm_abs_f32 (src, dest, (uint32_t) num);
    }

    template <typename Size>
    void abs (double* dest, const double* src, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] = std::abs (src[i]);
    }

    template <typename Size>
    void min (float* dest, const float* src, float comp, Size num) noexcept
    {
        scalar_min_cmp (dest, src, comp, num);
    }

    template <typename Size>
    void min (double* dest, const double* src, double comp, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] = std::min (src[i], comp);
    }

    template <typename Size>
    void min (float* dest, const float* src1, const float* src2, Size num) noexcept
    {
        scalar_min_elem (dest, src1, src2, num);
    }

    template <typename Size>
    void min (double* dest, const double* src1, const double* src2, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] = std::min (src1[i], src2[i]);
    }

    template <typename Size>
    void max (float* dest, const float* src, float comp, Size num) noexcept
    {
        scalar_max_cmp (dest, src, comp, num);
    }

    template <typename Size>
    void max (double* dest, const double* src, double comp, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] = std::max (src[i], comp);
    }

    template <typename Size>
    void max (float* dest, const float* src1, const float* src2, Size num) noexcept
    {
        scalar_max_elem (dest, src1, src2, num);
    }

    template <typename Size>
    void max (double* dest, const double* src1, const double* src2, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
            dest[i] = std::max (src1[i], src2[i]);
    }

    template <typename Size>
    void clip (float* dest, const float* src, float low, float high, Size num) noexcept
    {
        scalar_clip (dest, src, low, high, num);
    }

    template <typename Size>
    void clip (double* dest, const double* src, double low, double high, Size num) noexcept
    {
        for (Size i = 0; i < num; ++i)
        {
            double x = src[i];
            if (x < low)
                x = low;
            if (x > high)
                x = high;
            dest[i] = x;
        }
    }

    template <typename Size>
    float findMinimum (const float* src, Size num) noexcept
    {
        if (num <= 0)
            return 0.0f;
        float minVal;
        uint32_t idx;
        arm_min_f32 (src, (uint32_t) num, &minVal, &idx);
        return minVal;
    }

    template <typename Size>
    double findMinimum (const double* src, Size num) noexcept
    {
        if (num <= 0)
            return 0.0;
        double mn = src[0];
        for (Size i = 1; i < num; ++i)
            if (src[i] < mn)
                mn = src[i];
        return mn;
    }

    template <typename Size>
    float findMaximum (const float* src, Size num) noexcept
    {
        if (num <= 0)
            return 0.0f;
        float maxVal;
        uint32_t idx;
        arm_max_f32 (src, (uint32_t) num, &maxVal, &idx);
        return maxVal;
    }

    template <typename Size>
    double findMaximum (const double* src, Size num) noexcept
    {
        if (num <= 0)
            return 0.0;
        double mx = src[0];
        for (Size i = 1; i < num; ++i)
            if (src[i] > mx)
                mx = src[i];
        return mx;
    }

    template <typename Size>
    void convertFixedToFloat (float* dest, const int* src, float multiplier, Size num) noexcept
    {
        // Generic int32 -> float scale (fast enough on M7F)
        for (Size i = 0; i < num; ++i)
            dest[i] = (float) src[i] * multiplier;
    }

} // namespace FloatVectorHelpers
} // namespace chowdsp_juce

//==============================================================================
// Public API specialisations, identical signatures/names as your original file.
namespace chowdsp_juce
{
namespace detail
{
    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::clear (FloatType* dest, CountType num) noexcept
    {
        FloatVectorHelpers::clear (dest, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::fill (FloatType* dest, FloatType v, CountType num) noexcept
    {
        FloatVectorHelpers::fill (dest, v, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::copy (FloatType* dest, const FloatType* src, CountType num) noexcept
    {
        std::memcpy (dest, src, (size_t) num * sizeof (FloatType));
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::copyWithMultiply (FloatType* dest, const FloatType* src, FloatType k, CountType num) noexcept
    {
        FloatVectorHelpers::copyWithMultiply (dest, src, k, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::add (FloatType* dest, FloatType amt, CountType num) noexcept
    {
        FloatVectorHelpers::add (dest, amt, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::add (FloatType* dest, const FloatType* src, FloatType amt, CountType num) noexcept
    {
        FloatVectorHelpers::add (dest, src, amt, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::add (FloatType* dest, const FloatType* src, CountType num) noexcept
    {
        FloatVectorHelpers::add (dest, src, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::add (FloatType* dest, const FloatType* a, const FloatType* b, CountType num) noexcept
    {
        FloatVectorHelpers::add (dest, a, b, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::subtract (FloatType* dest, const FloatType* src, CountType num) noexcept
    {
        FloatVectorHelpers::subtract (dest, src, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::subtract (FloatType* dest, const FloatType* a, const FloatType* b, CountType num) noexcept
    {
        FloatVectorHelpers::subtract (dest, a, b, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::addWithMultiply (FloatType* dest, const FloatType* src, FloatType k, CountType num) noexcept
    {
        FloatVectorHelpers::addWithMultiply (dest, src, k, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::addWithMultiply (FloatType* dest, const FloatType* a, const FloatType* b, CountType num) noexcept
    {
        FloatVectorHelpers::addWithMultiply (dest, a, b, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::subtractWithMultiply (FloatType* dest, const FloatType* src, FloatType k, CountType num) noexcept
    {
        FloatVectorHelpers::subtractWithMultiply (dest, src, k, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::subtractWithMultiply (FloatType* dest, const FloatType* a, const FloatType* b, CountType num) noexcept
    {
        FloatVectorHelpers::subtractWithMultiply (dest, a, b, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::multiply (FloatType* dest, const FloatType* src, CountType num) noexcept
    {
        FloatVectorHelpers::multiply (dest, src, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::multiply (FloatType* dest, const FloatType* a, const FloatType* b, CountType num) noexcept
    {
        FloatVectorHelpers::multiply (dest, a, b, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::multiply (FloatType* dest, FloatType k, CountType num) noexcept
    {
        FloatVectorHelpers::multiply (dest, k, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::multiply (FloatType* dest, const FloatType* src, FloatType k, CountType num) noexcept
    {
        FloatVectorHelpers::multiply (dest, src, k, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::negate (FloatType* dest, const FloatType* src, CountType num) noexcept
    {
        FloatVectorHelpers::negate (dest, src, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::abs (FloatType* dest, const FloatType* src, CountType num) noexcept
    {
        FloatVectorHelpers::abs (dest, src, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::min (FloatType* dest, const FloatType* src, FloatType c, CountType num) noexcept
    {
        FloatVectorHelpers::min (dest, src, c, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::min (FloatType* dest, const FloatType* a, const FloatType* b, CountType num) noexcept
    {
        FloatVectorHelpers::min (dest, a, b, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::max (FloatType* dest, const FloatType* src, FloatType c, CountType num) noexcept
    {
        FloatVectorHelpers::max (dest, src, c, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::max (FloatType* dest, const FloatType* a, const FloatType* b, CountType num) noexcept
    {
        FloatVectorHelpers::max (dest, a, b, num);
    }

    template <typename FloatType, typename CountType>
    void JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::clip (FloatType* dest, const FloatType* src, FloatType lo, FloatType hi, CountType num) noexcept
    {
        FloatVectorHelpers::clip (dest, src, lo, hi, num);
    }

    template <typename FloatType, typename CountType>
    FloatType JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::findMinimum (const FloatType* src, CountType num) noexcept
    {
        return FloatVectorHelpers::findMinimum (src, num);
    }

    template <typename FloatType, typename CountType>
    FloatType JUCE_CALLTYPE FloatVectorOperationsBase<FloatType, CountType>::findMaximum (const FloatType* src, CountType num) noexcept
    {
        return FloatVectorHelpers::findMaximum (src, num);
    }

    // Explicit instantiations to mirror the original
    template struct FloatVectorOperationsBase<float, int>;
    template struct FloatVectorOperationsBase<float, size_t>;
    template struct FloatVectorOperationsBase<double, int>;
    template struct FloatVectorOperationsBase<double, size_t>;

} // namespace detail

void JUCE_CALLTYPE FloatVectorOperations::convertFixedToFloat (float* dest, const int* src, float multiplier, size_t num) noexcept
{
    FloatVectorHelpers::convertFixedToFloat (dest, src, multiplier, num);
}

void JUCE_CALLTYPE FloatVectorOperations::convertFixedToFloat (float* dest, const int* src, float multiplier, int num) noexcept
{
    FloatVectorHelpers::convertFixedToFloat (dest, src, multiplier, num);
}

intptr_t JUCE_CALLTYPE FloatVectorOperations::getFpStatusRegister() noexcept
{
    intptr_t fpsr = 0;
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__) || defined(__ARM_FEATURE_DSP)
    asm volatile ("vmrs %0, fpscr" : "=r"(fpsr));
#endif
    return fpsr;
}

void JUCE_CALLTYPE FloatVectorOperations::setFpStatusRegister (intptr_t fpsr) noexcept
{
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__) || defined(__ARM_FEATURE_DSP)
    asm volatile ("vmsr fpscr, %0" :: "r"(fpsr));
#else
    (void) fpsr;
#endif
}

void JUCE_CALLTYPE FloatVectorOperations::enableFlushToZeroMode (bool shouldEnable) noexcept
{
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__) || defined(__ARM_FEATURE_DSP)
    constexpr intptr_t mask = (1 << 24);
    const auto current = getFpStatusRegister();
    setFpStatusRegister ((current & ~mask) | (shouldEnable ? mask : 0));
#else
    (void) shouldEnable;
#endif
}

void JUCE_CALLTYPE FloatVectorOperations::disableDenormalisedNumberSupport (bool shouldDisable) noexcept
{
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__) || defined(__ARM_FEATURE_DSP)
    enableFlushToZeroMode (shouldDisable);
#else
    (void) shouldDisable;
#endif
}

bool JUCE_CALLTYPE FloatVectorOperations::areDenormalsDisabled() noexcept
{
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__) || defined(__ARM_FEATURE_DSP)
    constexpr intptr_t mask = (1 << 24);
    return (getFpStatusRegister() & mask) == mask;
#else
    return false;
#endif
}

ScopedNoDenormals::ScopedNoDenormals() noexcept
{
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__) || defined(__ARM_FEATURE_DSP)
    constexpr intptr_t mask = (1 << 24);
    fpsr = FloatVectorOperations::getFpStatusRegister();
    FloatVectorOperations::setFpStatusRegister (fpsr | mask);
#endif
}

ScopedNoDenormals::~ScopedNoDenormals() noexcept
{
#if defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7M__) || defined(__ARM_FEATURE_DSP)
    FloatVectorOperations::setFpStatusRegister (fpsr);
#endif
}

void JUCE_CALLTYPE FloatVectorOperations_convertFixedToFloat_f32_sz (float* dest, const int* src, float multiplier, size_t num) noexcept
{
    FloatVectorOperations::convertFixedToFloat (dest, src, multiplier, num);
}

void JUCE_CALLTYPE FloatVectorOperations_convertFixedToFloat_f32_i (float* dest, const int* src, float multiplier, int num) noexcept
{
    FloatVectorOperations::convertFixedToFloat (dest, src, multiplier, num);
}

} // namespace chowdsp_juce
