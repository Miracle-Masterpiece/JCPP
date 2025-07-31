#include <allocators/arena_free_list_allocator.hpp>


namespace tca
{

    arena_free_list_allocator::arena_free_list_allocator(std::size_t arena_size, base_allocator* allocator) :
    base_allocator(), 
    m_arena(internal::chunk::make(allocator, arena_size)) {
        
    }

    arena_free_list_allocator::~arena_free_list_allocator() {

    }

    void* arena_free_list_allocator::allocate(std::size_t sz) {
        return allocate_align(sz, alignof(std::max_align_t));
    }
    
    void* arena_free_list_allocator::allocate_align(std::size_t sz, std::size_t align) {
        return m_arena.allocate_first_fit(sz);
    }
    
    void arena_free_list_allocator::deallocate(void* p, std::size_t sz) {
        m_arena.free(p);
    }
    
    arena_free_list_allocator::arena_free_list_allocator(arena_free_list_allocator&& alloc) :
    base_allocator(std::move(alloc)), 
    m_arena(std::move(alloc.m_arena)) {

    }

    arena_free_list_allocator& arena_free_list_allocator::operator= (arena_free_list_allocator&& alloc) {
        if (&alloc != this) {
            base_allocator::operator=(std::move(alloc));
            m_arena = std::move(alloc.m_arena);
        }
        return *this;
    }

    void arena_free_list_allocator::reset() {
        assert(m_arena.is_empty());
        m_arena.reset();
    }

    void arena_free_list_allocator::print() const {
        m_arena.print_log();
    }

    void arena_free_list_allocator::compact() {
        m_arena.compact();
    }
    
}