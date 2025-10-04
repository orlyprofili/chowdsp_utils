floatvecotoperations for m7f:

JUCE’s `FloatVectorOperationsBase` is written as a set of platform-specific SIMD intrinsics with fallbacks.

* On **Cortex-A (application cores)**: it uses NEON intrinsics for acceleration. That’s the main optimized path.
* On **Cortex-M (microcontroller cores, e.g. M4/M7/M33)**: those do not have NEON. They only have the Armv7-M or Armv8-M scalar FPU (single-precision, optional double on some M7/M55). `FloatVectorOperationsBase` will compile, but it will fall back to plain C loops or whatever scalar ops JUCE defines. No SIMD.
* So: it is *compatible* with Cortex-M (will run correctly), but only **optimized** for cores with NEON (Cortex-A, some R).

If you want efficiency on Cortex-M, you’d need to write CMSIS-DSP or hand-tuned FPU/vector code. JUCE does not provide M-class FPU/Helium (M55/M85) paths at present.

Summary:

* **Cortex-A** → accelerated (NEON).
* **Cortex-M** → functional, but scalar only (no vectorization).

To get CMSIS-DSP or M7F-specific optimizations requires a custom version of JUCE’s FloatVectorOperationsBase.

Notes:

CMSIS has no fused vector FMA for scalar constants. Scalar loops for multiplyAdd are fine on M7F with I-cache warm.

If you need double paths, keep JUCE’s scalar for double and specialize only float.

For Helium (M55/M85), you can replace these with arm_*_f32 Helium variants from CMSIS 6.0+ when available.

Build config:

CPU/FPU: -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard

Opt: -O3 or -Ofast (if acceptable), -fno-math-errno -fno-trapping-math

Defines: -DARM_MATH_CM7 -DARM_MATH_MATRIX_CHECK=0 -DARM_MATH_ROUNDING=0

Linker: enable I-cache + D-cache in startup; place hot code in ITCM if available.

JUCE wiring:

Exclude juce_core/maths/juce_FloatVectorOperations.cpp from the M7F target.

Add the replacement juce_FloatVectorOperations.cpp file instead. No other JUCE changes.

Runtime FP setup (flush denormals, lazy stacking):

# include "cmsis_gcc.h"
# include "core_cm7.h"

static inline void enable_fp_controls(void)
{
    // ASPEN|LSPEN = lazy stacking; FZ = flush-to-zero; DN = default NaN
    FPU->FPCCR |= FPU_FPCCR_ASPEN_Msk | FPU_FPCCR_LSPEN_Msk | FPU_FPCCR_FZ_Msk | FPU_FPCCR_DN_Msk;
    __DSB();__ISB();
}


Call once early (after clocks, before DSP use).

Data layout tips:

4-byte align float buffers; 16-byte align if you can.

Process long vectors in blocks of 64–256 samples to keep the D-cache happy.

Avoid aliasing: keep dest != src unless the API states in-place is supported.

When to prefer CMSIS vs scalar:

copy/fill/add/sub/mul/scale/dot/abs/neg → CMSIS wins on M7F.

multiplyAdd and mixed ops → scalar loop in the provided file is fine; avoid extra temp buffers.

Sanity check:

Build with -ffunction-sections -fdata-sections and link with --gc-sections.

Verify FP instructions present: objdump -d | grep -E "v(f|mul|add|sub|mla)".

Future, NOT NOW: add int16_t/q15_t paths for mixed fixed-point audio on M7F
