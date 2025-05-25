#ifndef _ALLOCATORS_LINEAR_ALLOCATOR_H
#define _ALLOCATORS_LINEAR_ALLOCATOR_H

#include "./base_allocator.hpp"

namespace tca {


class linear_allocator : public base_allocator {
    void*       _buffer;
    std::size_t _capacity;
    std::size_t _offset;
    linear_allocator(const linear_allocator&);
    linear_allocator& operator= (const linear_allocator&);
public:
    linear_allocator();    
    linear_allocator(void* baseBuffer, std::size_t capacity);
    linear_allocator(base_allocator* baseAllocator, std::size_t capacity);
    linear_allocator(linear_allocator&&);
    linear_allocator& operator= (linear_allocator&&);
    ~linear_allocator();
    template<typename T>
    T* allocate(std::size_t count);
    void* allocate(std::size_t sz) override;
    void* allocate_align(std::size_t sz, std::size_t align) override;
    void  deallocate(void* ptr, std::size_t sz) override;
    std::size_t to_string(char buf[], std::size_t buf_size) const;
    void reset() {
        _offset = 0;
    }
    std::size_t position() {
        return _offset;
    }
    void print() const;
};
    template<typename T>
    T* linear_allocator::allocate(std::size_t count) {
        return reinterpret_cast<T*>(allocate_align(sizeof(T) * count, alignof(T)));
    }
}


#endif//_ALLOCATORS_LINEAR_ALLOCATOR_H
