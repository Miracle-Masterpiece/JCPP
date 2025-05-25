#include <cpp/lang/utils/shared_ptr.hpp>
#include <cassert>

namespace jstd
{

namespace internal
{

namespace sptr
{

        /**
         * tca::base_allocator*    m_allocator;
         * void*                   m_object;
         * uint32_t                m_strong_refs;
         * uint32_t                m_weak_refs;
         */

        shared_control_block::shared_control_block() : 
        m_allocator(nullptr), m_object(nullptr), m_strong_refs(0), m_weak_refs(0) {

        }

        shared_control_block::shared_control_block(tca::base_allocator* allocator, void* object) :
        m_allocator(allocator), m_object(object), m_strong_refs(0), m_weak_refs(0) {

        }

        shared_control_block::~shared_control_block() {

        }

        void shared_control_block::inc_strong_ref() {
            ++m_strong_refs;
        }

        void shared_control_block::dec_strong_ref() {
            assert(m_strong_refs > 0);
            --m_strong_refs;
        }

        void shared_control_block::inc_weak_ref() {
            ++m_weak_refs;
        }
        
        void shared_control_block::dec_weak_ref() {
            assert(m_weak_refs > 0);
            --m_weak_refs;
        }

        uint32_t shared_control_block::strong_count() const {
            return m_strong_refs;
        }
        
        uint32_t shared_control_block::weak_count() const {
            return m_weak_refs;
        }

        int32_t shared_control_block::to_string(char buf[], int32_t bufsize) const {
            return snprintf(buf, bufsize, "[data=0x%llx, strong=%llu, weak=%llu]", (unsigned long long) m_object, (unsigned long long) m_strong_refs, (unsigned long long) m_weak_refs);
        }

}// namespace sptr 

}// namespace internal 

}// namespace jstd