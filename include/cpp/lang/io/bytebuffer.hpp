#ifndef ALLOCATORS_LANG_IO_BYTEBUFFER_H_
#define ALLOCATORS_LANG_IO_BYTEBUFFER_H_

#include <allocators/base_allocator.hpp>
#include <cpp/lang/system.hpp>
#include <cpp/lang/utils/utils.hpp>
#include <cstdint>
#include <cstring>
#include <climits>

namespace jstd {

class byte_buffer {
    tca::base_allocator*    _allocator;
    char*                   _data;
    int64_t             _cap;
    int64_t             _pos;
    int64_t             _limit;
    int64_t             _mark;
    byte_order              _order;
    void dispose();
    void checkIndex(int64_t idx, int64_t sz) const;
public:
    byte_buffer();
    byte_buffer(char* buf, int64_t bufsize);
    explicit byte_buffer(int64_t capacity, tca::base_allocator* allocator = tca::get_scoped_or_default());
    byte_buffer(const byte_buffer&);
    byte_buffer(byte_buffer&&);
    byte_buffer& operator= (const byte_buffer&);
    byte_buffer& operator= (byte_buffer&&);
    ~byte_buffer();
    static byte_buffer allocate(int64_t capacity, tca::base_allocator* allocator = tca::get_scoped_or_default());
    
    int64_t position() const;
    byte_buffer& position(int64_t newpos);

    int64_t limit() const;
    byte_buffer& limit(int64_t newlimit);

    int64_t remaining() const;

    int64_t capacity() const;

    byte_buffer& clear();
    byte_buffer& compact();
    byte_buffer& flip();
    byte_buffer& mark();

    byte_order order() const;
    byte_buffer& order(byte_order order);

    char* data();
    const char* data() const;

    template<typename T>
    byte_buffer& put(T v);
    
    template<typename T>
    byte_buffer& put(int64_t idx, T v);

    template<typename T>
    byte_buffer& puts(const T* arr, int64_t sz);

    template<typename T>
    byte_buffer& puts(int64_t idx, const T* arr, int64_t sz);
    
    template<typename T>
    T get();

    template<typename T>
    T get(int64_t idx) const;

    template<typename T>
    byte_buffer& gets(T* arr, int64_t sz);

    template<typename T>
    byte_buffer& gets(int64_t idx, T* arr, int64_t sz);

    void print() const;

    int to_string(char buf[], int bufsize) const;
};

    template<typename T>
    byte_buffer& byte_buffer::put(T v) {
        put<T>(_pos, v);
        _pos += sizeof(v);
        return *this;
    }
    
    template<typename T>
    byte_buffer& byte_buffer::put(int64_t idx, T v) {
        checkIndex(idx, sizeof(v));
        if (_order != system::native_byte_order()) {
            utils::copy_swap_memory(_data + idx, reinterpret_cast<const char*>(&v), sizeof(v), 1);
        } else {
            std::memcpy(_data + idx, reinterpret_cast<const char*>(&v), sizeof(v));
        }
        return *this;
    }

    template<typename T>
    byte_buffer& byte_buffer::puts(const T* arr, int64_t sz) {
        puts<T>(_pos, arr, sz);
        _pos += sizeof(T) * sz;
        return *this;
    }

    template<typename T>
    byte_buffer& byte_buffer::puts(int64_t idx, const T* arr, int64_t sz) {
        checkIndex(idx, sizeof(T) * sz);
        if (_order != system::native_byte_order()) {
            utils::copy_swap_memory(_data + idx, reinterpret_cast<const char*>(arr), sizeof(T), sz);
        } else {
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
        T ret;
        if (_order != system::native_byte_order()) {
            utils::copy_swap_memory(reinterpret_cast<char*>(&ret), _data + idx, sizeof(ret), 1);
        } else {
            std::memcpy(reinterpret_cast<char*>(&ret), _data + idx, sizeof(ret));
        }
        return ret;
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
            utils::copy_swap_memory(reinterpret_cast<char*>(arr), _data + idx, sizeof(T), sz);
        } else {
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