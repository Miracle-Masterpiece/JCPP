#include <allocators/pool_allocator.hpp>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <new>
#include <cstdint>

#ifndef NDEBUG
    #define tca_check_currupt_memblock(block)   if (block->m_magic != memblock::MAGIC) {            \
                                                    std::printf("pool_allocator currupt!\n");       \
                                                    abort();                                        \
                                                }                                                                                 
#else
    #define tca_check_currupt_memblock(block)
#endif

namespace tca {

namespace internal {


    /**
     * 
     *  base_allocator*         m_allocator;
     *  void*                   m_data;
     *  std::size_t             m_byte_size;
     *  std::size_t             m_bucket_size;
     *  std::size_t             m_bucket_count;
     *  memblock*               m_freelist;
     * 
     */

    void pool::link(memblock* block) {
        assert(block != nullptr);
        block->m_next   = m_freelist;
        m_freelist      = block;
    }

    pool::memblock* pool::unlink() {
        memblock* block = m_freelist;
        if (block == nullptr)
            return nullptr;
        tca_check_currupt_memblock(block);
        m_freelist      = m_freelist->m_next;
        return block;
    }

    pool::pool() : 
    m_allocator(nullptr), 
    m_data(nullptr), 
    m_byte_size(0), 
    m_bucket_size(0), 
    m_bucket_count(0), 
    m_freelist(nullptr) {

    }
    
    pool::pool(base_allocator* allocator, std::size_t blocksize, std::size_t count_blocks) : pool() {
        blocksize = calcAlignSize(blocksize, alignof(std::max_align_t));
        void* data = allocator->allocate_align((blocksize + HEADER_SIZE) * count_blocks, alignof(std::max_align_t));
        if (data != nullptr) {
            m_allocator     = allocator;
            m_data          = data;
            m_byte_size     = (blocksize + HEADER_SIZE) * count_blocks;
            m_bucket_size   = blocksize + HEADER_SIZE;
            m_bucket_count  = count_blocks;
            m_freelist      = nullptr;
            
            for (std::size_t i = 0; i < count_blocks; ++i) {
                std::size_t offset = i * m_bucket_size;
                memblock* block = new(reinterpret_cast<char*>(data) + offset) memblock();
                link(block);
            }

        }
    }
    
    /**
     * 
     *  base_allocator*         m_allocator;
     *  void*                   m_data;
     *  std::size_t             m_byte_size;
     *  std::size_t             m_bucket_size;
     *  std::size_t             m_bucket_count;
     *  memblock*               m_freelist;
     * 
     */

    pool::pool(pool&& p) :
    m_allocator(p.m_allocator),
    m_data(p.m_data),
    m_byte_size(p.m_byte_size),
    m_bucket_size(p.m_bucket_size),
    m_bucket_count(p.m_bucket_count),
    m_freelist(p.m_freelist) {
        p.m_allocator       = nullptr;
        p.m_data            = nullptr;
        p.m_byte_size       = 0;
        p.m_bucket_size     = 0;
        p.m_bucket_count    = 0;
        p.m_freelist        = nullptr;
        update_owner(&p);
    }

    pool& pool::operator= (pool&& p) {
        if (&p != this) {
            cleanup();
            m_allocator     = p.m_allocator;
            m_data          = p.m_data;
            m_byte_size     = p.m_byte_size;
            m_bucket_size   = p.m_bucket_size;
            m_bucket_count  = p.m_bucket_count;
            m_freelist      = p.m_freelist;
            
            p.m_allocator       = nullptr;
            p.m_data            = nullptr;
            p.m_byte_size       = 0;
            p.m_bucket_size     = 0;
            p.m_bucket_count    = 0;
            p.m_freelist        = nullptr;

            update_owner(&p);
        }
        return *this;
    }
    
    void pool::cleanup() {
        if (m_allocator != nullptr && m_data != nullptr) {
            m_allocator->deallocate(m_data, m_byte_size);
            m_data      = nullptr;
            m_allocator = nullptr;
        }
    }

    void pool::update_owner(const pool* old_owner) {
        assert(old_owner != nullptr);
        void* data = m_data;
        for (std::size_t i = 0; i < m_bucket_count; ++i) {
            std::size_t offset = i * m_bucket_size;
            memblock* block = reinterpret_cast<memblock*>(reinterpret_cast<char*>(data) + offset);
            if (block->m_owner == old_owner)
                block->m_owner = this;
        }
    }

    pool::~pool() {
        cleanup();
    }
    
    void* pool::allocate() {
        memblock* block = unlink();
        if (block == nullptr)
            return nullptr;
        block->m_owner = this;
        return reinterpret_cast<void*>(reinterpret_cast<char*>(block) + HEADER_SIZE);
    }
    
    void pool::deallocate(void* p) {
        if (p == nullptr)
            return;
        memblock* block = void_to_memblock(p);
        tca_check_currupt_memblock(block);
        link(block);
    }

}//namespace internal
































    pool_allocator::pool_allocator() : m_allocator(nullptr), m_pool(), m_pool_size(0) {

    }

    pool_allocator::pool_allocator(base_allocator* allocator, std::size_t size) : 
    m_allocator(allocator), m_pool(allocator), m_pool_size(size) {

    }
    
    // pool_allocator(pool_allocator&&);
    // pool_allocator& operator= (pool_allocator&&);
    
    pool_allocator::~pool_allocator() {

    }

    void* pool_allocator::allocate(std::size_t sz) {
        return allocate_align(sz, alignof(std::max_align_t));
    }

    void* pool_allocator::allocate_align(std::size_t sz, std::size_t align /*ingnored*/) {
        assert(sz <= m_pool_size);
        void* p = nullptr;
        for (uint64_t i = 0; i < m_pool.size(); ++i) {
            internal::pool& pool = m_pool.at(i);
            p = pool.allocate();
            if (p != nullptr)
                return p;
        }

        {
            internal::pool pool(m_allocator, m_pool_size, COUNT_POOL_BUCKETS);
            p = pool.allocate();
            if (p != nullptr)
                m_pool.add(std::move(pool));
        }

        return p;
    }
    
    void pool_allocator::deallocate(void* p, std::size_t sz) {
        if (p == nullptr)
            return;
        internal::pool::memblock* memblock = internal::pool::void_to_memblock(p);
        internal::pool* pool = memblock->m_owner;
        pool->deallocate(p);
    }


}//namespace tca