#ifndef ALLOCATORS_INLINE_LINEAR_ALLOCATOR_ALLOCATOR_H
#define ALLOCATORS_INLINE_LINEAR_ALLOCATOR_ALLOCATOR_H

#include <allocators/linear_allocator.hpp>
#include <cstddef>

namespace tca {

template<std::size_t SIZE>
class inline_linear_allocator : public linear_allocator {
    inline_linear_allocator(const inline_linear_allocator<SIZE>&)                   = delete;
    inline_linear_allocator<SIZE>& operator=(const inline_linear_allocator<SIZE>&)  = delete;
    inline_linear_allocator(inline_linear_allocator<SIZE>&&)                        = delete;
    inline_linear_allocator<SIZE>& operator=(inline_linear_allocator<SIZE>&&)       = delete;
public:
    union {
        char _buf[SIZE];
        std::max_align_t _align;
    } _data;
    inline_linear_allocator();
    ~inline_linear_allocator();
};
    
    template<std::size_t SIZE>
    inline_linear_allocator<SIZE>::inline_linear_allocator() : linear_allocator(_data._buf, SIZE) {

    }
    
    template<std::size_t SIZE>
    inline_linear_allocator<SIZE>::~inline_linear_allocator() {

    }
}
#endif//ALLOCATORS_INLINE_LINEAR_ALLOCATOR_ALLOCATOR_H