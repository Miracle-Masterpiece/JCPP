#include <allocators/base_allocator.hpp>
#include <cstdlib>
#include <cstdio>

namespace tca{

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
}