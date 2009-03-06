
#ifndef cuda_processor_h__
#define cuda_processor_h__ 1

#include "cuda_util.h"

void emdata_processor_mult( EMDataForCuda* cuda_data, const float& mult);

void emdata_processor_correlation_texture( const EMDataForCuda* left);

void emdata_processor_correlation( const EMDataForCuda* left, const EMDataForCuda* right);

#endif // cuda_processor_h__