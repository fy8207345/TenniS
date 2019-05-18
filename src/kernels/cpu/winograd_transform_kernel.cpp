#include <kernels/cpu/winograd_transform_kernel.h>

#include "kernels/cpu/winograd_transform_kernel.h"
#include "global/operator_factory.h"
#include "backend/name.h"
#include <kernels/cpu/conv2d_algorithm.h>

namespace ts {
    namespace cpu {
        void WinogradTransKernel::transform_kernel(const Tensor &x, WinogradConv2DModel winograd_model, Tensor &out) {
            DTYPE dtype = out.dtype();
            switch (dtype) {
#define DECLARE_COMPUTE_RUN(DTYPE, TYPE) \
            case DTYPE: { \
                if (winograd_model == F6X6_3X3) \
                    Conv2dAlgorithm<TYPE>::conv3x3_winograd63_transform_kernel_inplace(x, out); \
                else if(winograd_model == F2X2_3X3) \
                    Conv2dAlgorithm<TYPE>::conv3x3_winograd23_transform_kernel_inplace(x, out); \
                break; }
                DECLARE_COMPUTE_RUN(FLOAT32, float);
                DECLARE_COMPUTE_RUN(FLOAT64, double);
#undef DECLARE_COMPUTE_RUN
            default: {
                TS_LOG_ERROR << this->op() << " not support this data type: " << dtype << eject;
                break;
                }
            }
        }
    }
}

using namespace ts;
using namespace cpu;
TS_REGISTER_OPERATOR(WinogradTransKernel, ts::CPU, name::layer::winograd_transform_kernel())