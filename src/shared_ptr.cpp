#include <cpp/lang/utils/shared_ptr.hpp>
#include <allocators/Helpers.hpp>

namespace jstd
{
namespace internal
{
namespace sptr 
{
    uint32_t calc_ctr_block_total_size(uint32_t data_type_sizeof, uint32_t data_type_alignof, uint32_t* offset_to_object) {
        const uint32_t ctrl_block_sz            = sizeof(shared_control_block);
        const uint32_t padding                  = tca::calcAlignAddedSize(ctrl_block_sz, data_type_alignof);
        const uint32_t ctrl_block_total_size    = ctrl_block_sz + padding + data_type_sizeof;
        if (offset_to_object != nullptr)
            (*offset_to_object) = (ctrl_block_sz + padding);
        return ctrl_block_total_size;
    }

    shared_control_block* alloc_memory_to_control_block(tca::base_allocator* allocator, std::size_t object_size, std::size_t object_align, std::size_t n_objects) {        
        using u32 = uint32_t;
        u32 ctrl_block_size     = sizeof(shared_control_block);
        u32 padding             = tca::calcAlignAddedSize(ctrl_block_size, object_align);
        u32 offset_to_object    = ctrl_block_size + padding;
        u32 objects_size        = object_size * n_objects;
        u32 total_size          = ctrl_block_size + padding + objects_size;
        void* p                 = allocator->allocate_align(total_size, 
                                                object_align > alignof(shared_control_block) ? 
                                                object_align : alignof(shared_control_block));         
        if (!p)
            return nullptr;

        using byte_t = unsigned char;
        byte_t* block = reinterpret_cast<byte_t*>(p);
        shared_control_block* ctrl_block = reinterpret_cast<shared_control_block*>(block); {
            ctrl_block->m_allocator     = allocator;
            ctrl_block->m_blocksize     = total_size;
            ctrl_block->m_object        = block + offset_to_object;
            ctrl_block->m_strong_refs   = 0;
            ctrl_block->m_weak_refs     = 0;
        }
        return ctrl_block;
    }

}// namespace sptr 
}// namespace internal
}// namespace jstd