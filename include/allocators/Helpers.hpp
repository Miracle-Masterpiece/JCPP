#ifndef ALLOCATORS_HELPERS_H
#define ALLOCATORS_HELPERS_H

#include <cctype>
#include <cstdint>

namespace tca
{
    
    constexpr inline std::size_t calc_padding_for(void* p, std::size_t align) {
        //std::size_t mod = size % align;
        //return mod == 0 ? 0 : align - mod;
        return (intptr_t) p % align == 0 ? 0 : align - ((intptr_t) p % align);
    }
    
    constexpr inline std::size_t calcAlignAddedSize(std::size_t size, std::size_t align) {
        //std::size_t mod = size % align;
        //return mod == 0 ? 0 : align - mod;
        return size % align == 0 ? 0 : align - (size % align);
    }
    
    constexpr inline std::size_t calcAlignSize(std::size_t size, std::size_t align) {
        return size + calcAlignAddedSize(size, align);
    }
    
    constexpr inline std::size_t calc_aligned_size(std::size_t size, std::size_t align) {
        return size + calcAlignAddedSize(size, align);
    }
}

#endif//ALLOCATORS_HELPERS_H