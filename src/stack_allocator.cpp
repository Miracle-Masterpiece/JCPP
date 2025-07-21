#include <allocators/stack_allocator.hpp>

namespace tca {
    stack_frame::stack_frame(stack_allocator* stack) : stack(stack) {
        
    }
    
    stack_frame::stack_frame(stack_frame&& sf) : stack(sf.stack) {
        sf.stack = nullptr;
    }
    
    stack_frame& stack_frame::operator=(stack_frame&& sf) {
        if (&sf != this) {
            stack       = sf.stack;
            sf.stack    = nullptr;
        } 
        return *this;
    }

    stack_frame::~stack_frame() {
        stack->pop_frame();
    }

    stack_allocator::stack_allocator(std::size_t size, base_allocator* allocator) : 
    base_allocator(allocator), 
    m_data(nullptr), 
    m_data_length(0), 
    m_last_frame(nullptr), 
    m_stack_size(0), 
    m_total_allocated(0) {
        void* p                 = NULL;
        std::size_t length      = 0;
        {
            length  = size;
            p       = allocator->allocate_align(length, alignof(stack_frame_t));
        }
        m_data               = ((char*) p); 
        m_data_length        = length;
        m_stack_size         = size;
    }

    stack_allocator::~stack_allocator(){
        if (m_parent != nullptr)
            m_parent->deallocate(m_data, m_data_length);
    }

    stack_frame stack_allocator::push_frame() {
        const std::size_t FRAME_SIZE   = sizeof(stack_frame_t);
        std::size_t allocated          = m_total_allocated;
        std::size_t stack_size         = m_stack_size;

        stack_frame_t* old_frame    = m_last_frame;
        stack_frame_t* new_frame    = nullptr; 
        
        std::size_t padding;
        {
            char* start    = (m_data + m_total_allocated);
            padding         = calcAlign(start, sizeof(stack_frame_t));
            new_frame       = (stack_frame_t*) (start + padding);
            if (allocated + FRAME_SIZE + padding > stack_size)
                return stack_frame(nullptr);
        }
        
        #ifndef NDEBUG
            assert(((intptr_t) new_frame % sizeof(stack_frame_t*)) == 0);
        #endif

        new_frame->m_prev           = old_frame;
        new_frame->m_allocated      = padding;
        m_total_allocated   += alignof_size_frame() + padding;
        m_last_frame         = new_frame;
        
        return stack_frame(this);
    }

    bool stack_allocator::pop_frame() {
        stack_frame_t* frame = m_last_frame;
        if (frame == NULL)
            return false;
        std::size_t total_allocated  = frame->m_allocated + alignof_size_frame();
        m_total_allocated           -= total_allocated;
        m_last_frame                 = frame->m_prev;
        return true;
    }

    void* stack_allocator::alloc_bytes(std::size_t size, std::size_t align) {
        stack_frame_t* frame = m_last_frame;
        assert(frame != NULL);
        char*  start        = m_data + m_total_allocated;
        std::size_t padding = calcAlign(start, align);
        std::size_t total   = size + padding;
        if ((m_stack_size - m_total_allocated) < total + padding)
            throw std::bad_alloc();
        void* aligned_address   = (void*) (start + padding);
        frame->m_allocated      += total;
        m_total_allocated       += total;
        return aligned_address;
    }

    void* stack_allocator::allocate(std::size_t sz) {
        return alloc_bytes(sz, alignof(char));
    }
    
    void* stack_allocator::allocate_align(std::size_t sz, std::size_t align) {
        return alloc_bytes(sz, align);
    }
    
    void stack_allocator::deallocate(void* p, std::size_t sz) {

    }

    void stack_allocator::frame_print(stack_frame_t* frame) const {
        printf("[allocated: %zu | prev: %p]\n", frame->m_allocated, (void*) frame->m_prev);
    }

    void stack_allocator::print() const {
        printf("[allocated: %zu | free: %zu]\n", m_total_allocated, m_stack_size - m_total_allocated);
    }
}