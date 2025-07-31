#ifndef ALLOC_LIB_POOL_ALLOCATOR_H
#define ALLOC_LIB_POOL_ALLOCATOR_H
#include <allocators/base_allocator.hpp>
#include <allocators/Helpers.hpp>
#include <allocators/ArrayList.h> 
#include <cstddef>

namespace tca {

namespace internal {

/**
 * 
 */
class pool : public base_allocator {
public:
    struct memblock {
/**
 * Для проверки повреждения памяти.
 */
#ifndef NDEBUG
        static const unsigned long long int MAGIC = 0xCAFEBABEDEADULL;
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

    /**
     * 
     */
    base_allocator* m_allocator;
    
    /**
     * 
     */
    void* m_data;
    
    /**
     * 
     */
    std::size_t m_byte_size;
    
    /**
     * 
     */
    std::size_t m_bucket_size;
    
    /**
     * 
     */
    std::size_t m_bucket_count;
    
    /**
     * 
     */
    memblock* m_freelist;
    
    /**
     * 
     */
    std::size_t m_allocated;

    /**
     * 
     */
    pool(const pool&) = delete;
    
    /**
     * 
     */
    pool& operator= (const pool&) = delete;

    /**
     * 
     */
    void link(memblock*);
    
    /**
     * 
     */
    memblock* unlink();

    /**
     * 
     */
    void update_owner(const pool* old_owner); 
    
    /**
     * 
     */
    void cleanup();

public:
    using base_allocator::allocate;
    using base_allocator::deallocate;

    /**
     * 
     */
    pool();
    
    /**
     * 
     */
    pool(std::size_t blocksize, std::size_t count_blocks, base_allocator* allocator = get_scoped_or_default());
    
    /**
     * 
     */
    pool(pool&&);
    
    /**
     * 
     */
    pool& operator= (pool&&);
    
    /**
     * 
     */
    ~pool();
    
    /**
     * 
     */
    void* allocate();
    
    /**
     * 
     */
    void  deallocate(void*);
    
    /**
     * 
     */
    std::size_t is_free() const {
        return m_allocated == 0;
    }
    
    /**
     * 
     */
    void* allocate(std::size_t sz) override {
        assert(sz <= m_bucket_size);
        return allocate();
    }

    /**
     * 
     */
    void* allocate_align(std::size_t sz, std::size_t) override {
        assert(sz <= m_bucket_size);
        return allocate();
    }

    /**
     * 
     */
    void deallocate(void* p, std::size_t) override {
        deallocate(p);
    }

    /**
     * 
     */
    static std::size_t byte_size_for_pool(std::size_t block_size, std::size_t count_buckets) {
        return (block_size + HEADER_SIZE) * count_buckets;
    }

    /**
     * 
     */
    static memblock* void_to_memblock(void* p) {
        return reinterpret_cast<memblock*>(reinterpret_cast<char*>(p) - HEADER_SIZE);
    }
};

}

/**
 * Пул-аллокатор.
 * Выделяет блоки фиксированного размера.
 * Аллокатор не освобождает память, если пул свободен. 
 * Для освобождения необходимо вызвать функцию {pool_allocator::free_unsused_pools()}
 * 
 * @since 1.0
 */
class pool_allocator : public base_allocator {
public:
    /**
     * 
     */
    static const std::size_t DEFAULT_COUNT_BUCKETS = 1024;
private:
    /**
     * 
     */
    tca::base_allocator* m_allocator;

    /**
     * 
     */
    array_list<internal::pool>  m_pool;

    /**
     * 
     */
    std::size_t m_count_buckets;
    
    /**
     * 
     */
    std::size_t m_pool_size;

    /**
     * 
     */
    pool_allocator(const pool_allocator&)               = delete;
    
    /**
     * 
     */
    pool_allocator& operator= (const pool_allocator&)   = delete;

public:
    using base_allocator::allocate;
    using base_allocator::deallocate;
    
    /**
     * @param size
     *      Размер каждой ячейки в пуле. (В sizeof(char))
     * 
     * @param buckets_count
     *      Количество корзин в пуле.
     * 
     * @param allocator
     *      Распределитель памяти для пула.
     * 
     */
    explicit pool_allocator(std::size_t size, std::size_t buckets_count = DEFAULT_COUNT_BUCKETS, base_allocator* allocator = get_scoped_or_default());
    
    /**
     * 
     */
    pool_allocator(pool_allocator&&);
    
    /**
     * 
     */
    pool_allocator& operator= (pool_allocator&&);
    
    /**
     * 
     */
    ~pool_allocator();
    
    /**
     * 
     */
    void* allocate();
    
    /**
     * 
     */
    void deallocate(void* p);
    
    /**
     * 
     */
    void* allocate(std::size_t) override;
    
    /**
     * 
     */
    void* allocate_align(std::size_t, std::size_t) override;
    
    /**
     * 
     */
    void deallocate(void*, std::size_t) override;

    /**
     * Освобождает память неиспользованных пулов.
     */
    void free_unsused_pools();
};

typedef internal::pool fixed_pool_allocator;

}

#endif//ALLOC_LIB_POOL_ALLOCATOR_H