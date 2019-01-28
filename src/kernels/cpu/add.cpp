#include <kernels/cpu/add.h>
#include <core/tensor_builder.h>
#include <backend/name.h>
#include <utils/assert.h>
#include <global/operator_factory.h>
#include <core/device.h>

namespace ts {
namespace cpu {


//////////////////////////////////////////////
Add::Add() {
}

static inline int to_mod_index(const HypeShape &hype, const std::vector<int> &coordinate) {
    auto temp = coordinate;
    for (size_t i = 0; i < temp.size(); ++i) {
        temp[i] %= hype.shape(i);
    }
    return hype.to_index(temp);
}

template<typename T>
static inline void compute_run(const Tensor &lhs, const Tensor &rhs, Tensor &out) {
    HypeShape lhs_hype(lhs.sizes());
    HypeShape rhs_hype(rhs.sizes());
    HypeShape out_hype(out.sizes());

    auto plhs = lhs.data<T>();
    auto prhs = rhs.data<T>();
    auto pout = out.data<T>();

    auto ncount = out.count();
    for(int i = 0; i < ncount; i++) {
        std::vector<int> tmpshape = out_hype.to_coordinate(i);
        pout[i] = plhs[to_mod_index(lhs_hype, tmpshape)] + prhs[to_mod_index(rhs_hype,tmpshape)];
    }
}


template<typename T>
static inline void compute_run_scalar(const Tensor &lhs, const Tensor &rhs, Tensor &out) {
    auto plhs = lhs.data<T>();
    auto prhs = rhs.data<T>();
    auto pout = out.data<T>();

    auto scalar = prhs[0];
    auto ncount = out.count();

    // this is CPU operator, so just using memcpy
    std::memcpy(pout, plhs, ncount * sizeof(T));

    for (int i = 0; i < ncount;++i) {
        pout[i] += scalar;
    }
}


template<typename T>
static inline void compute_run_same_shape(const Tensor &lhs, const Tensor &rhs, Tensor &out) {
    auto plhs = lhs.data<T>();
    auto prhs = rhs.data<T>();
    auto pout = out.data<T>();

    auto ncount = out.count();

    // this is CPU operator, so just using memcpy
    std::memcpy(pout, plhs, ncount * sizeof(T));

    for (int i = 0; i < ncount;++i) {
        pout[i] += prhs[i];
    }
}



void Add::reduce_with_broadcast(const Tensor &lhs, const Tensor &rhs, Tensor &out) {
    // Notice: the all tensor' memory device are CPU, as given in running_memory_device
    DTYPE dtype = out.dtype();
    switch(dtype) {
#define DECLARE_COMPUTE_RUN(DTYPE, TYPE) \
        case DTYPE: { compute_run<TYPE>(lhs, rhs, out); break; }
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
            TS_LOG_ERROR << "add not support this data type: " << dtype << eject;
            break;
        }
    }
}

    void Add::reduce_with_scalar(const Tensor &lhs, const Tensor &rhs, Tensor &out) {
        // Notice: the all tensor' memory device are CPU, as given in running_memory_device
        DTYPE dtype = out.dtype();
        switch(dtype) {
#define DECLARE_COMPUTE_RUN(DTYPE, TYPE) \
        case DTYPE: { compute_run_scalar<TYPE>(lhs, rhs, out); break; }
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
                TS_LOG_ERROR << "add not support this data type: " << dtype << eject;
                break;
            }
        }
    }

    void Add::reduce_with_bias(const Tensor &lhs, const Tensor &rhs, Tensor &out, int dim) {
        // Notice: the all tensor' memory device are CPU, as given in running_memory_device
        supper::reduce_with_bias(lhs, rhs, out, dim);
    }

    void Add::reduce_with_same_shape(const Tensor &lhs, const Tensor &rhs, Tensor &out) {
        // Notice: the all tensor' memory device are CPU, as given in running_memory_device
        DTYPE dtype = out.dtype();
        switch(dtype) {
#define DECLARE_COMPUTE_RUN(DTYPE, TYPE) \
        case DTYPE: { compute_run_same_shape<TYPE>(lhs, rhs, out); break; }
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
                TS_LOG_ERROR << "add not support this data type: " << dtype << eject;
                break;
            }
        }
    }

    MemoryDevice Add::running_memory_device() {
        return MemoryDevice(CPU);
    }

}
}

using namespace ts;
using namespace cpu;
TS_REGISTER_OPERATOR(Add, CPU, name::layer::add())
