#include <allocators/base_allocator.hpp>
#include <allocators/malloc_free_allocator.hpp>
#include <cstdlib>
#include <cstdio>
#include <cassert>

namespace tca{

namespace internal 
{
    thread_local base_allocator* scoped_allocator = nullptr;
}

    
    base_allocator* get_default_allocator() {
        static malloc_free_allocator s_malloc_allocator;
        return &s_malloc_allocator;
    }

    base_allocator* get_scoped_or_default() {
        return internal::scoped_allocator != nullptr ? internal::scoped_allocator : get_default_allocator();
    }

    base_allocator::base_allocator() : parent(nullptr) {

    }

    base_allocator::base_allocator(base_allocator* parent) : parent(parent) {
        
    }

    base_allocator::base_allocator(base_allocator&& base) : parent(base.parent) {
        base.parent = nullptr;
    }

    base_allocator& base_allocator::operator= (base_allocator&& base) {
        if (&base != this) {
            parent      = base.parent;
            base.parent = nullptr;
        }
        return *this;
    }

    base_allocator::~base_allocator() {

    }

    scope_allocator::scope_allocator(base_allocator* allocator) : m_prev(internal::scoped_allocator) {
        assert(allocator != nullptr);
        internal::scoped_allocator = allocator;
    }
    
    scope_allocator::~scope_allocator() {
        if (internal::scoped_allocator != nullptr) {
            internal::scoped_allocator = m_prev;
        }
    }
    
    base_allocator* scope_allocator::get_prev() const {
        return m_prev;
    }
}