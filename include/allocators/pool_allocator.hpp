#ifndef ALLOC_LIB_POOL_ALLOCATOR_H
#define ALLOC_LIB_POOL_ALLOCATOR_H
#include <allocators/base_allocator.hpp>
#include <allocators/Helpers.hpp>
#include <allocators/ArrayList.h> 
#include <cstddef>

namespace tca {


namespace internal {

class pool {
public:
    struct memblock {
/**
 * Для проверки повреждения памяти.
 */
#ifndef NDEBUG
        static const unsigned long long int MAGIC = 0xCAFEBABEDEAD;
        unsigned long long int m_magic; //  Если данное поле не равно MAGIC, значит память была повреждена!
#endif//NDEBUG
        union {
            memblock*   m_next;
            pool*       m_owner;
        };
        memblock() : m_owner(nullptr) {
#ifndef NDEBUG
            m_magic = MAGIC;
#endif//NDEBUG
        }
    };
private:
    static const std::size_t HEADER_SIZE = calcAlignSize(sizeof(memblock), alignof(std::max_align_t));

    base_allocator*         m_allocator;
    void*                   m_data;
    std::size_t             m_byte_size;
    std::size_t             m_bucket_size;
    std::size_t             m_bucket_count;
    memblock*               m_freelist;

    pool(const pool&) = delete;
    pool& operator= (const pool&) = delete;

    void link(memblock*);
    memblock* unlink();

    void update_owner(const pool* old_owner); 
    void cleanup();

public:
    pool();
    pool(base_allocator* allocator, std::size_t blocksize, std::size_t count_blocks);
    pool(pool&&);
    pool& operator= (pool&&);
    ~pool();
    void* allocate();
    void  deallocate(void*);
    static memblock* void_to_memblock(void* p) {
        return reinterpret_cast<memblock*>(reinterpret_cast<char*>(p) - HEADER_SIZE);
    }
};

}

class pool_allocator : public base_allocator {
    pool_allocator(const pool_allocator&)               = delete;
    pool_allocator& operator= (const pool_allocator&)   = delete;
    
    static const std::size_t    COUNT_POOL_BUCKETS = 1024;
    tca::base_allocator*        m_allocator;
    array_list<internal::pool>  m_pool;
    std::size_t                 m_pool_size;

public:
    using base_allocator::allocate;
    using base_allocator::deallocate;
    explicit pool_allocator(std::size_t size, base_allocator* allocator = get_scoped_or_default());
    pool_allocator(pool_allocator&&);
    pool_allocator& operator= (pool_allocator&&);
    ~pool_allocator();
    void* allocate();
    void deallocate(void* p);
    void* allocate(std::size_t) override;
    void* allocate_align(std::size_t, std::size_t) override;
    void deallocate(void*, std::size_t) override;
};

}

#endif//ALLOC_LIB_POOL_ALLOCATOR_H