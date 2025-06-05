#include <allocators/shared_allocator.hpp>
#include <allocators/os_allocator.hpp>
#include <exception>
#include <iostream>

namespace tca {
using namespace internal;


    chunk::chunk() : 
    data_(nullptr), 
    chunk_size_(0), 
    freelist_(nullptr), 
    allocator_(nullptr),
    allocs_(0) {

    }

    chunk::chunk(byte_t* block, std::size_t sz, base_allocator* allocator) : 
    data_(block), 
    chunk_size_(sz), 
    freelist_(nullptr), 
    allocator_(allocator),
    allocs_(0)  {
        if (sz >= HEADER_SIZE) {
            reset();
        } 

        else {
            cleaunup();
        }
    }

    chunk::chunk(chunk&& ch) : chunk() {
        data_       = ch.data_;
        chunk_size_ = ch.chunk_size_;
        freelist_   = ch.freelist_;
        allocator_  = ch.allocator_;
        allocs_     = ch.allocs_;
        update_this_poitner(&ch);

        ch.data_        = nullptr;
        ch.chunk_size_  = 0;
        ch.freelist_    = nullptr;
        ch.allocator_   = nullptr;
        ch.allocs_      = 0;
    }

    chunk& chunk::operator= (chunk&& ch) {
        if (&ch != this) {
            cleaunup();
            data_           = ch.data_;
            chunk_size_     = ch.chunk_size_;
            freelist_       = ch.freelist_;
            allocator_      = ch.allocator_;
            allocs_         = ch.allocs_;
            update_this_poitner(&ch);

            ch.data_        = nullptr;
            ch.chunk_size_  = 0;
            ch.freelist_    = nullptr;
            ch.allocator_   = nullptr;
            ch.allocs_      = 0;
        }
        return *this;
    }

    void chunk::update_this_poitner(chunk* old_owner) {
        memblock* start = reinterpret_cast<memblock*>(data_);
        memblock* end   = reinterpret_cast<memblock*>(reinterpret_cast<byte_t*>(data_) + chunk_size_);
        for (memblock* block = start; block < end; ) {
            if (block->owner_ == old_owner)
                block->owner_ = this;
            block = reinterpret_cast<memblock*>(reinterpret_cast<byte_t*>(block) + HEADER_SIZE + block->len_);
        }
    }

    void chunk::cleaunup() {
        if (allocator_ != nullptr && data_ != nullptr) {
            allocator_->deallocate(data_, chunk_size_);
            allocator_  = nullptr;
            data_       = nullptr;
            chunk_size_ = 0;
            freelist_   = nullptr;
            allocs_     = 0;
        }
    }

    chunk::~chunk() {
        cleaunup();
    }

    void chunk::link(memblock* block) {
        assert(block != nullptr);
        
#ifndef NDEBUG
        assert(block->magic_ == memblock::DEBUG_NUMBER);
#endif

        block->prev_ = nullptr;
        block->next_ = freelist_;
        
        if (freelist_ != nullptr)
            freelist_->prev_ = block;
        
        freelist_ = block;
    }
    
    void chunk::unlink(memblock* block) {
        assert(block != nullptr);

#ifndef NDEBUG
        assert(block->magic_ == memblock::DEBUG_NUMBER);
#endif

        memblock* prev = block->prev_;
        memblock* next = block->next_;

        if (prev != nullptr) {
            prev->next_ = next;
        } else {
            freelist_ = next;
        }

        if (next != nullptr) {
            next->prev_ = prev;
        }
    }
    
    memblock* chunk::split(memblock* block, std::size_t sz) {

        const int64_t MIN_SIZE_FOR_SPLIT = static_cast<int64_t>(HEADER_SIZE + MIN_ALLOC_SIZE);
        
        int64_t total_size  = static_cast<int64_t>(block->len_);
        int64_t size        = static_cast<int64_t>(sz);

        if (total_size < MIN_SIZE_FOR_SPLIT || total_size - size < MIN_SIZE_FOR_SPLIT)
            return block;

        byte_t* block_byte_view = reinterpret_cast<byte_t*>(block); 
            
        byte_t* next_byte_view  = block_byte_view + HEADER_SIZE + sz;
        memblock* next = new(next_byte_view) memblock();
        
        next->len_      = total_size - size - HEADER_SIZE;
        //next->is_free   = true;
        
        next->next_     = block->next_;
        if (next->next_ != nullptr)
            next->next_->prev_ = next;
        next->prev_ = block;
        
        block->next_ = next;
        block->len_ = sz;
        
        return block;
    }

    /*static*/ chunk chunk::make(base_allocator* allocator, std::size_t sz) {
        byte_t* mem = reinterpret_cast<byte_t*>(allocator->allocate_align(sz, ALIGN));
        if (mem == nullptr)
            return chunk();
        return chunk(mem, sz, allocator);
    }

    void* chunk::allocate_first_fit(std::size_t sz) {
        if (sz > 0)
            sz = calcAlignSize(sz, ALIGN);
        else
            sz = ALIGN;

        bool _try_again = true;
        AGAIN:
        for (memblock* block = freelist_; block != nullptr; block = block->next_) {
            if (block->len_ >= sz) {
                if (block->len_ > sz)
                    block = split(block, sz);
                //block->is_free = false;
                unlink(block);
                block->owner_ = this;
                ++allocs_;
                return reinterpret_cast<void*>(reinterpret_cast<byte_t*>(block) + HEADER_SIZE);
            }
        }
        if (_try_again ) {
            _try_again = false;
            compact();
            goto AGAIN;
        }
        return nullptr;
    }
    
    void* chunk::allocate_best_fit(std::size_t sz) {
        if (sz > 0)
            sz = calcAlignSize(sz, ALIGN);
        else
            sz = ALIGN;
        
        bool _try_again = true;
        AGAIN:

        memblock* best_block = nullptr;
        for (memblock* block = freelist_; block != nullptr; block = block->next_) {
            if (block->len_ >= sz) {
                
                if (best_block == nullptr) {
                    best_block = block;
                } 
                
                else if (block->len_ == sz) {
                    best_block = block;
                    break;
                }

                else if (block->len_ < best_block->len_) {
                    best_block = block;
                }
            }
        }
        if (best_block == nullptr) {
            if (_try_again) {
                compact();
                _try_again = false;
                goto AGAIN;
            }
            return nullptr;
        }
        //best_block->is_free = false;

        if (best_block->len_ > sz)
            best_block = split(best_block, sz);

        unlink(best_block);
        best_block->owner_ = this;
        ++allocs_;
        return reinterpret_cast<void*>(reinterpret_cast<byte_t*>(best_block) + HEADER_SIZE);
    }
    
    void chunk::reset() {
        const std::size_t OFFSET = HEADER_SIZE;
        const std::size_t free_size = chunk_size_ - OFFSET;
        freelist_           = new(data_) memblock();
        freelist_->len_     = free_size;
        freelist_->next_    = nullptr;
        freelist_->prev_    = nullptr;
    }

    void chunk::compact() {
        memblock* start = reinterpret_cast<memblock*>(data_);
        memblock* end   = reinterpret_cast<memblock*>(reinterpret_cast<byte_t*>(data_) + chunk_size_);
        for (memblock* block = start; block < end; ) {
            if (block->owner_ != this) {
                memblock* next = reinterpret_cast<memblock*>(reinterpret_cast<byte_t*>(block) + HEADER_SIZE + block->len_);
                if (next < end) {
                    if (next->owner_ != this) {
                        unlink(block);
                        unlink(next);
                        block->len_ += HEADER_SIZE + next->len_;
                        link(block);
                        continue;
                    }
                }    
            }
            block = reinterpret_cast<memblock*>(reinterpret_cast<byte_t*>(block) + HEADER_SIZE + block->len_);
        }
    }
    
    void chunk::free(void* p) {
        if (p == nullptr)
            return;
        memblock* block = reinterpret_cast<memblock*>(reinterpret_cast<byte_t*>(p) - HEADER_SIZE);
#ifndef NDEBUG
        if (block->magic_ != memblock::DEBUG_NUMBER) {
            std::printf("shared_allocator heap corrupted!\n");
            exit(-1);
        }
#endif
        //block->is_free = true;  
        --allocs_;
        link(block);
    }

    bool chunk::is_empty() const {
        return allocs_ == 0;
    }

    void chunk::print_log() const {
        std::printf("=================== F R E E  L I S T ===================\n=================== allocs: %lli ===================\n", (long long int) allocs_);
        for (memblock* block = freelist_; block != nullptr; block = block->next_) {
            #ifndef NDEBUG
                std::printf("[p:%p, sz:%zu, magic: %llX]\n", block, block->len_, block->magic_);
            #else
                std::printf("[p:%p, sz:%zu]\n", block, block->len_);
            #endif
        }

        std::printf("=================== T O T A L  B L O C K S ===================\n");
        memblock* start = reinterpret_cast<memblock*>(data_);
        memblock* end   = reinterpret_cast<memblock*>(reinterpret_cast<byte_t*>(data_) + chunk_size_);
        for (memblock* block = start; block < end; ) {
            #ifndef NDEBUG
                std::printf("[p:%p, sz:%zu, f:%s, magic: %llX]\n", block, block->len_, block->owner_ != this ? "true" : "false", block->magic_);
            #else
                std::printf("[p:%p, f:%s, sz:%zu]\n", block, block->owner_ != this ? "true" : "false", block->len_);
            #endif
            block = reinterpret_cast<memblock*>(reinterpret_cast<byte_t*>(block) + HEADER_SIZE + block->len_);
        }
    }


    /**
     * #########################################################################################################
     * #########################################################################################################
     */

     shared_allocator::shared_allocator() : allocator_(nullptr), strategy_(alloc_strategy::FIRST_FIT) {

    }

    shared_allocator::shared_allocator(base_allocator* allocator, alloc_strategy strategy_) :
    shared_allocator(allocator, DEFAULT_MIN_CHUNK_SIZE, strategy_)
    {

    }

    shared_allocator::shared_allocator(base_allocator* allocator, std::size_t min_chunk_aligned, alloc_strategy strategy) :
    allocator_(allocator),
    chunk_list_(allocator),
    min_chunk_aligned_(min_chunk_aligned),
    strategy_(strategy) {

    }

    shared_allocator::shared_allocator(shared_allocator&& sa) : shared_allocator() {
        allocator_      = sa.allocator_;
        chunk_list_     = std::move(sa.chunk_list_);
        strategy_       = sa.strategy_;
        sa.allocator_   = nullptr;
    }
    
    shared_allocator& shared_allocator::operator= (shared_allocator&& sa) {
        if (&sa != this) {
            allocator_      = sa.allocator_;
            chunk_list_     = std::move(sa.chunk_list_);
            strategy_       = sa.strategy_;
            sa.allocator_   = nullptr;
        }
        return *this;
    }
    
    shared_allocator::~shared_allocator() {

    }

    void shared_allocator::set_alloc_type(shared_allocator::alloc_strategy type) {
        strategy_ = type;
    }
    
    shared_allocator::alloc_strategy shared_allocator::get_alloc_type() const {
        return strategy_;
    }

    void* shared_allocator::allocate_align(std::size_t sz, std::size_t align) {
        return allocate(sz);
    }

    void* shared_allocator::allocate(std::size_t sz) {
        jstd::unique_lock lock(m_global_lock);
        void* p = nullptr;
        for (int32_t i = 0, size = chunk_list_.size(); i < size; ++i) {
            chunk& ch =  chunk_list_.at(i);
            if (strategy_ == alloc_strategy::BEST_FIT) {
                p = ch.allocate_best_fit(sz);
            } else {
                p = ch.allocate_first_fit(sz);
            }
            if (p != nullptr)
                break;
        }

        if (p == nullptr) {
            chunk _new = chunk::make(allocator_, calcAlignSize(sz + chunk::HEADER_SIZE, min_chunk_aligned_));
            if (_new.data_ != nullptr) {
                p = _new.allocate_first_fit(sz);
                if (p != nullptr) {
                    chunk_list_.add(std::move(_new));
                    return p;
                }
            }
        }

        return p;
    }
    
    void shared_allocator::release_unused() {
        jstd::unique_lock lock(m_global_lock);
        for (int64_t i = 0; i < (int64_t) chunk_list_.size(); ++i) {
            chunk& c = chunk_list_.at(i);
            if (c.is_empty())
                chunk_list_.remove_at(i--);
        } 
    }

    void shared_allocator::merge_free_blocks() {
        jstd::unique_lock lock(m_global_lock);
        for (int64_t i = 0, size = chunk_list_.size(); i < size; ++i) {
            chunk_list_.at(i).compact();
        } 
    }

    void shared_allocator::deallocate(void* p, std::size_t sz) {
        if (p == nullptr)
            return;
        jstd::unique_lock lock(m_global_lock);
#ifdef NDEBUG
        memblock* block = chunk::data_to_mem(p);
        chunk* c = block->owner_;
        c->free(p);
        if (c->is_empty()) {
            for (int64_t i = 0, size = chunk_list_.size(); i < size; ++i) {
                if (&chunk_list_.at(i) == c) {
                    chunk_list_.remove_at(i);
                    break;
                }
            } 
        }
#else        
        memblock* block = chunk::data_to_mem(p); 
        for (int64_t i = 0, size = chunk_list_.size(); i < size; ++i) {
            chunk& c = chunk_list_.at(i);
            void* start = reinterpret_cast<void*>(c.data_);
            void* end   = reinterpret_cast<void*>(c.data_ + c.chunk_size_);
            if (p >= start && p < end) {
                if (block->owner_ != &c) {
                    std::printf("shared_allocator: double free detected!\n");
                    std::abort();
                } else {
                    c.free(p);
                    //if (c.is_empty())
                    //   chunk_list_.remove_at(i);
                }
                break;
            }
        }
#endif
    }

    void shared_allocator::print_log() const {
        m_global_lock.lock();
        for (int64_t i = 0, size = chunk_list_.size(); i < size; ++i) {
            chunk_list_.at(i).print_log();
            std::printf("\n\n");
        }
        m_global_lock.unlock();
    }
}