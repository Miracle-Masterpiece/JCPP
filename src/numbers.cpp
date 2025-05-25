#include <cpp/lang/numbers.hpp>
#include <cstring>


namespace jstd {
namespace num {
    int32_t float_to_int_bits(float v) {
        static_assert(sizeof(v) == sizeof(int32_t), "float must be 32_bit");
        int32_t value;
        std::memcpy(&value, &v, sizeof(float));
        return value;
    }
    
    int64_t double_to_long_bits(double v) {
        static_assert(sizeof(v) == sizeof(int64_t), "double must be 64_bit");
        int64_t value;
        std::memcpy(&value, &v, sizeof(double));
        return value;
    }
}
}

