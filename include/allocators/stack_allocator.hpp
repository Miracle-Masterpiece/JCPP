#ifndef _ALLOCATORS_STACK_ALLOCATOR_H
#define _ALLOCATORS_STACK_ALLOCATOR_H

#include <allocators/base_allocator.hpp>
#include <cinttypes>
#include <cassert>
#include <cstdio>
#include <exception>

namespace tca {

struct stack_frame_t {
    stack_frame_t*      m_prev;
    std::size_t         m_allocated;
};

class stack_allocator;
class stack_frame {
    stack_allocator* stack;
    stack_frame(const stack_frame&);
    stack_frame& operator= (const stack_frame&);
public:
    stack_frame(stack_allocator* stack);
    stack_frame(stack_frame&&);
    stack_frame& operator=(stack_frame&&);
    ~stack_frame();
};

class stack_allocator : public base_allocator {
    char*           m_data;
    size_t          m_data_length;
    stack_frame_t*  m_last_frame;

    std::size_t     m_stack_size;
    std::size_t     m_total_allocated;
public:
    stack_allocator(base_allocator* allocator, std::size_t size);

    ~stack_allocator();

    inline std::size_t calcAlign(void* ptr, std::size_t align){
        const intptr_t p            = (intptr_t) ptr;
        const std::size_t mod       = p & (align - 1);
        return mod == 0 ? 0 : align - mod;
    }

    inline std::size_t calc_align_size(std::size_t size, std::size_t align){
        const std::size_t p  = size;
        const std::size_t mod  = p & (align - 1);
        return mod == 0 ? 0 : align - mod;
    }
    
    inline std::size_t alignof_size_frame() {
        return sizeof(stack_frame_t) + calc_align_size(sizeof(stack_frame_t), alignof(stack_frame_t));
    }

    stack_frame push_frame();
    bool pop_frame();
    void* alloc_bytes(std::size_t size, std::size_t align);
    void* allocate(std::size_t sz) override;
    void* allocate_align(std::size_t sz, std::size_t align) override;
    void deallocate(void* p, std::size_t sz) override;
    void frame_print(stack_frame_t* frame) const;
    void print() const;

};
    
}

#endif//_ALLOCATORS_STACK_ALLOCATOR_H