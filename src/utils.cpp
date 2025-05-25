#include <cpp/lang/utils/utils.hpp>
#include <cstring>

namespace jstd::utils {

    void copy_swap_memory(void* d, const void* s, std::size_t sz) {
        char* dst       = reinterpret_cast<char*>(d);
        const char* src = reinterpret_cast<const char*>(s);
        std::size_t off = 0;
        for (std::size_t j = sz - 1; ;--j) {
            dst[off++] = src[j];
            if (j == 0)
                break;
        }
    }

    void copy_swap_memory(void* dst, const void* src, std::size_t n, std::size_t count) {
        char*           d = reinterpret_cast<char*>(dst);
        const char*     s = reinterpret_cast<const char*>(src);
        for (std::size_t i = 0, len = n * count; i < len; i += n) {
            copy_swap_memory(d + i, s + i, n);
        }       
    }

    void swap(void* d, std::size_t sz) {
        char* data = reinterpret_cast<char*>(d);
        const std::size_t half_size = sz >> 1;
        std::size_t off       = sz - 1;
        for (std::size_t i = 0; i < half_size; ++i, --off) {
            const char a = data[i];
            const char b = data[off];
            data[i]     = b;
            data[off]   = a;
        }
    }
    
    void swap(void* dst, std::size_t sz, std::size_t count) {
        char* d = reinterpret_cast<char*>(dst);
        for (std::size_t i = 0, len = sz * count; i < len; i += sz) {
            swap(d + i, sz);
        }
    }
}