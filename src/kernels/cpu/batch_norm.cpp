#include <kernels/cpu/batch_norm.h>
#include <core/tensor_builder.h>

#include <global/operator_factory.h>
#include <backend/name.h>
#include <utils/assert.h>
#include <core/device.h>
#include <vector>

namespace ts {
    namespace cpu {

        template<typename T>
        static void cpu_batch_norm_compute_run(const Tensor &x, const Tensor &mean,
                                               const Tensor &variance, int dim, float epsilon, Tensor &out) {
            const Shape &shape = x.sizes();
            int predims = 1;
            int backdims = 1;
            for (int i = 0; i < dim; i++) {
                predims *= shape[i];
            }

            for (int i = dim + 1; i < shape.size(); i++) {
                backdims *= shape[i];
            }

            const T *psrc = x.data<T>();
            const T *pmean = mean.data<T>();
            const T *pvariance = variance.data<T>();
            T *pdst = out.data<T>();

            // only used in CPU
            std::memcpy(pdst, psrc, out.count() * sizeof(T));

            int stridedims = backdims * shape[dim];
            int offset = 0;

            std::vector<T> vec(variance.count());
            for (int i = 0; i < vec.size(); i++) {
                vec[i] = T(1) / sqrt(pvariance[i] + T(epsilon));
            }

            for (int i = 0; i < predims; i++) {
                for (int k = 0; k < shape[dim]; k++) {
                    offset = i * stridedims + k * backdims;
                    for (int m = 0; m < backdims; m++) {
                        pdst[offset + m] = (pdst[offset + m] - pmean[k]) * vec[k];//(sqrt(pvariance[k] + m_epsilon));
                    }
                }
            }
        }

        void BatchNorm::batch_norm(const Tensor &x, const Tensor &mean, const Tensor &variance,
                                   int dim, float epsilon, Tensor &out) {
            // Notice: the all tensor' memory device are CPU, as given in running_memory_device
            DTYPE dtype = out.dtype();
            switch (dtype) {
#define DECLARE_COMPUTE_RUN(DTYPE, TYPE) \
        case DTYPE: { cpu_batch_norm_compute_run<TYPE>(x, mean, variance, dim, epsilon, out); break; }
                DECLARE_COMPUTE_RUN(INT8, int8_t);
                DECLARE_COMPUTE_RUN(UINT8, uint8_t);
                DECLARE_COMPUTE_RUN(INT16, int16_t);
                DECLARE_COMPUTE_RUN(UINT16, uint16_t);
                DECLARE_COMPUTE_RUN(INT32, int32_t);
                DECLARE_COMPUTE_RUN(UINT32, uint32_t);
                DECLARE_COMPUTE_RUN(INT64, int64_t);
                DECLARE_COMPUTE_RUN(UINT64, uint64_t);
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
TS_REGISTER_OPERATOR(BatchNorm, CPU, name::layer::batch_norm())
