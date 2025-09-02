#ifndef ALLOCATORS_LANG_IO_BYTEBUFFER_H_
#define ALLOCATORS_LANG_IO_BYTEBUFFER_H_

#include <allocators/allocator.hpp>
#include <cpp/lang/system.hpp>
#include <cpp/lang/utils/utils.hpp>
#include <cpp/lang/utils/cond_compile.hpp>
#include <cstdint>
#include <cstring>
#include <climits>

namespace jstd {

JSTD_DEBUG_CODE(
    class index_out_of_bound_exception;
    class readonly_exception;
);

class byte_buffer {
protected:
    
    /**
     * 
     */
    tca::allocator*    _allocator;
    
    /**
     * 
     */
    char*                   _data;
    
    /**
     * 
     */
    int64_t                 _cap;
    
    /**
     * 
     */
    int64_t                 _pos;
    
    /**
     * 
     */
    int64_t                 _limit;
    
    /**
     * 
     */
    int64_t                 _mark;
    
    /**
     * 
     */
    byte_order              _order;
    
    /**
     * 
     */
    bool                    m_readonly;


    /**
     * deleted
     */
    byte_buffer(const byte_buffer&) = delete;

    /**
     * deleted
     */
    byte_buffer& operator= (const byte_buffer&) = delete;

    /**
     * 
     */
    void dispose();
    
    /**
     * 
     */
    void checkIndex(int64_t idx, int64_t sz) const {
        JSTD_DEBUG_CODE(
            if (idx > _limit) {
                throw_except<index_out_of_bound_exception>("Index %zu > _limit %zu", idx, _limit);
            }
            else if (_limit - idx < sz) {
                throw_except<index_out_of_bound_exception>("Remaining %zu < tsize %zu", _limit - idx, sz);
            }
        );
    }
    
    /**
     * 
     */
    void check_write_or_except() const {
        JSTD_DEBUG_CODE(
            if (m_readonly)
                throw_except<readonly_exception>("Buffer is read only");
        );
    }

public:
    /**
     * 
     */
    byte_buffer();
    
    /**
     * 
     */
    byte_buffer(char* buf, int64_t bufsize, bool readonly = false);
    
    /**
     * 
     */
    explicit byte_buffer(int64_t capacity, tca::allocator* allocator = tca::get_scoped_or_default());
    
    /**
     * 
     */
    byte_buffer(byte_buffer&&);
    
    /**
     * 
     */
    byte_buffer& operator= (byte_buffer&&);
    
    /**
     * 
     */
    ~byte_buffer();
    
    /**
     * 
     */
    static byte_buffer allocate(int64_t capacity, tca::allocator* allocator = tca::get_scoped_or_default());
    
    /**
     * 
     */
    int64_t position() const;
    
    /**
     * 
     */
    byte_buffer& position(int64_t newpos);

    /**
     * 
     */
    int64_t limit() const;
    
    /**
     * 
     */
    byte_buffer& limit(int64_t newlimit);

    /**
     * 
     */
    int64_t remaining() const;

    /**
     * 
     */
    int64_t capacity() const;

    /**
     * 
     */
    byte_buffer& clear();
    
    /**
     * 
     */
    byte_buffer& compact();
        
    /**
     * 
     */
    byte_buffer& flip();
        
    /**
     * 
     */
    byte_buffer& mark();
    
    /**
     * 
     */
    byte_order order() const;
        
    /**
     * 
     */
    byte_buffer& order(byte_order order);
    
    /**
     * 
     */
    char* data();
        
    /**
     * 
     */
    const char* data() const;
    
    /**
     * 
     */
    template<typename T>
    byte_buffer& put(T v);
        
    /**
     * 
     */
    template<typename T>
    byte_buffer& put(int64_t idx, T v);
    
    /**
     * 
     */
    template<typename T>
    byte_buffer& puts(const T* arr, int64_t sz);
    
    /**
     * 
     */
    template<typename T>
    byte_buffer& puts(int64_t idx, const T* arr, int64_t sz);
        
    /**
     * 
     */
    template<typename T>
    T get();
    
    /**
     * 
     */
    template<typename T>
    T get(int64_t idx) const;
    
    /**
     * 
     */
    template<typename T>
    byte_buffer& gets(T* arr, int64_t sz);
    
    /**
     * 
     */
    template<typename T>
    byte_buffer& gets(int64_t idx, T* arr, int64_t sz);
    
    /**
     * 
     */
    void print() const;
    
    /**
     * 
     */
    int to_string(char buf[], int bufsize) const;
};

    template<typename T>
    byte_buffer& byte_buffer::put(T v) {
        check_write_or_except();                                                                                                                            \
        put<T>(_pos, v);
        _pos += sizeof(v);
        return *this;
    }
    
    template<typename T>
    byte_buffer& byte_buffer::put(int64_t idx, T v) {
        check_write_or_except();                                                                                                                            \
        checkIndex(idx, sizeof(v));
        utils::write_with_order<T>(_data + idx, v, _order);
        return *this;
    }

    template<typename T>
    byte_buffer& byte_buffer::puts(const T* arr, int64_t sz) {
        check_write_or_except();                                                                                                                            \
        puts<T>(_pos, arr, sz);
        _pos += sizeof(T) * sz;
        return *this;
    }

    template<typename T>
    byte_buffer& byte_buffer::puts(int64_t idx, const T* arr, int64_t sz) {
        check_write_or_except();                                                                                                                            \
        checkIndex(idx, sizeof(T) * sz);
        if (_order != system::native_byte_order()) {
            utils::copy_swap_memory<T>(_data + idx, reinterpret_cast<const char*>(arr), sz);
        } 
        else {
            std::memcpy(_data + idx, reinterpret_cast<const char*>(arr), sizeof(T) * sz);
        }
        return *this;
    }
    
    template<typename T>
    T byte_buffer::get() {
        T tmp = get<T>(_pos);
        _pos += sizeof(tmp);
        return tmp;
    }

    template<typename T>
    T byte_buffer::get(int64_t idx) const {
        checkIndex(idx, sizeof(T));
        return utils::read_with_order<T>(_data + idx, _order);
    }

    template<typename T>
    byte_buffer& byte_buffer::gets(T* arr, int64_t sz) {
        gets<T>(_pos, arr, sz);
        _pos += sizeof(T) * sz;
        return *this;
    }

    template<typename T>
    byte_buffer& byte_buffer::gets(int64_t idx, T* arr, int64_t sz) {
        checkIndex(idx, sizeof(T) * sz);
        if (_order != system::native_byte_order()) {
            utils::copy_swap_memory<T>(reinterpret_cast<char*>(arr), _data + idx, sz);
        } 
        else {
            std::memcpy(reinterpret_cast<char*>(arr), _data + idx, sizeof(T) * sz);
        }
        return *this;
    }

    /**
     * ################################################################
     *              C H A R  S P E C I A L I Z A T I O N
     * ###############################################################
     */

#define ______MAKE____SPECIALIZATION____ALLOCATORS_LIB_BYTE_BUFFER(type_name__jstd_io_bytebuffer)                                                       \
                                                                                                                                                        \
    template<>                                                                                                                                          \
    byte_buffer& byte_buffer::put<type_name__jstd_io_bytebuffer>(type_name__jstd_io_bytebuffer v);                                                      \
                                                                                                                                                        \
    template<>                                                                                                                                          \
    byte_buffer& byte_buffer::put<type_name__jstd_io_bytebuffer>(int64_t idx, type_name__jstd_io_bytebuffer v);                                     \
                                                                                                                                                        \
    template<>                                                                                                                                          \
    byte_buffer& byte_buffer::puts<type_name__jstd_io_bytebuffer>(const type_name__jstd_io_bytebuffer* arr, int64_t sz);                            \
                                                                                                                                                        \
    template<>                                                                                                                                          \
    byte_buffer& byte_buffer::puts<type_name__jstd_io_bytebuffer>(int64_t idx, const type_name__jstd_io_bytebuffer* arr, int64_t sz);           \
                                                                                                                                                        \
    template<>                                                                                                                                          \
    type_name__jstd_io_bytebuffer byte_buffer::get<type_name__jstd_io_bytebuffer>();                                                                    \
                                                                                                                                                        \
    template<>                                                                                                                                          \
    type_name__jstd_io_bytebuffer byte_buffer::get<type_name__jstd_io_bytebuffer>(int64_t idx) const;                                               \
                                                                                                                                                        \
    template<>                                                                                                                                          \
    byte_buffer& byte_buffer::gets<type_name__jstd_io_bytebuffer>(type_name__jstd_io_bytebuffer* arr, int64_t sz);                                  \
                                                                                                                                                        \
    template<>                                                                                                                                          \
    byte_buffer& byte_buffer::gets<type_name__jstd_io_bytebuffer>(int64_t idx, type_name__jstd_io_bytebuffer* arr, int64_t sz);                 \

______MAKE____SPECIALIZATION____ALLOCATORS_LIB_BYTE_BUFFER(char)
______MAKE____SPECIALIZATION____ALLOCATORS_LIB_BYTE_BUFFER(signed char)
______MAKE____SPECIALIZATION____ALLOCATORS_LIB_BYTE_BUFFER(unsigned char)

#undef ______MAKE____SPECIALIZATION____ALLOCATORS_LIB_BYTE_BUFFER
}                                                                                                                                                   

#endif//ALLOCATORS_LANG_IO_BYTEBUFFER_H_