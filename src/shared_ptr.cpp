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
}// namespace sptr 
}// namespace internal
}// namespace jstd