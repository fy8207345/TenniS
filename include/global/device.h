//
// Created by lby on 2018/3/11.
//

#ifndef TENSORSTACK_GLOBAL_DEVICE_H
#define TENSORSTACK_GLOBAL_DEVICE_H

#include <string>
#include <ostream>
#include <utils/api.h>

namespace ts {
    /**
     * DeviceType: hardware includeing CPU, GPU or other predictable device
     */
    using DeviceType = std::string;
    static const char *CPU = "cpu";
    static const char *GPU = "gpu";
    static const char *EIGEN = "eigen";
    static const char *BLAS = "blas";
    static const char *CUDNN = "cudnn";

    /**
     * Device: Sepcific device
     */
    class Device {
    public:
        using self = Device;    ///< self class

        /**
         * Initialize device
         * @param type Hardware device @see Device
         * @param id Device type's id, 0, 1, or ...
         */
        Device(const DeviceType &type, int id) : m_type(type), m_id(id) {}

        /**
         * Initialize device
         * @param type Hardware device @see Device
         * @note Default id is 0
         */
        Device(const DeviceType &type) : self(type, 0) {}

        /**
         * Initialize device like CPU:0
         */
        Device() : self(CPU, 0) {}

        /**
         * Device type
         * @return Device type
         */
        const DeviceType &type() const { return m_type; }

        /**
         * Device id
         * @return Device id
         */
        int id() const { return m_id; }

        /**
         * return repr string
         * @return repr string
         */
        const std::string repr() const { return std::move(m_type + ":" + std::to_string(m_id)); }

        /**
         * return string show the content
         * @return string
         */
        const std::string str() const { return std::move(m_type + ":" + std::to_string(m_id)); }

    private:
        DeviceType m_type = CPU;  ///< Hardware device @see Device
        int m_id = 0; ///< Device type's id, 0, 1, or ...
    };

    inline std::ostream &operator<<(std::ostream &out, const Device &device) {
        TS_UNUSED(CPU);
        TS_UNUSED(GPU);
        TS_UNUSED(EIGEN);
        TS_UNUSED(BLAS);
        TS_UNUSED(CUDNN);
        return out << device.str();
    }

    bool operator==(const Device &lhs, const Device &rhs);

    bool operator!=(const Device &lhs, const Device &rhs);

    bool operator<(const Device &lhs, const Device &rhs);

    bool operator>(const Device &lhs, const Device &rhs);

    bool operator<=(const Device &lhs, const Device &rhs);

    bool operator>=(const Device &lhs, const Device &rhs);
}

#endif //TENSORSTACK_GLOBAL_DEVICE_H
