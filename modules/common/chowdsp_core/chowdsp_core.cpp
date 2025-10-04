#include "chowdsp_core.h"

#if ! CHOWDSP_USING_JUCE
#if ! defined(CHOWDSP_USE_CUSTOM_FLOAT_VECTOR_OPS)
#include "JUCEHelpers/juce_FloatVectorOperations.cpp"
#endif

#if ! JUCE_MODULE_AVAILABLE_juce_dsp
#include "JUCEHelpers/dsp/juce_LookupTable.cpp"
#endif
#endif
