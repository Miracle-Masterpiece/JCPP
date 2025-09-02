#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/numbers.hpp>
#include <cpp/lang/utils/comparator.hpp>

namespace jstd 
{
namespace objects 
{

    template<>
    uint64_t hashcode<float>(const float* array, int64_t len) {
#ifndef NDEBUG
        if (array == nullptr)
            throw_except<null_pointer_exception>("array must be != null");        
#endif//NDEBUG
        uint64_t hash = 0xcbf29ce484222325;

        if (len == -1) {
            while (*array) 
                hash = (hash ^ num::float_to_int_bits(*(array++))) * 0x100000001b3;
        } else {
            for (int64_t i = 0; i < len; ++i)
                hash = (hash ^ num::float_to_int_bits(array[i])) * 0x100000001b3;
        }

        return hash;
    }

    template<>
    uint64_t hashcode<double>(const double* array, int64_t len) {
#ifndef NDEBUG
        if (array == nullptr)
            throw_except<null_pointer_exception>("array must be != null");        
#endif//NDEBUG
        uint64_t hash = 0xcbf29ce484222325;

        if (len == -1) {
            while (*array) 
                hash = (hash ^ num::double_to_long_bits(*(array++))) * 0x100000001b3;
        } else {
            for (int64_t i = 0; i < len; ++i)
                hash = (hash ^ num::double_to_long_bits(array[i])) * 0x100000001b3;
        }

        return hash;
    }

}// namespace objects
}// namespace jstd 