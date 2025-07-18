#include <cpp/lang/io/bytebuffer.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cassert>
#include <exception>
#include <cstdio>

#define BYTE_BITS 8

namespace jstd {

    byte_buffer::byte_buffer() :
    _allocator(nullptr), 
    _data(nullptr), 
    _cap(0), 
    _pos(0), 
    _limit(0), 
    _mark(-1),
    _order(system::native_byte_order()),
    m_readonly(false) {

    }

    byte_buffer::byte_buffer(char* buf, int64_t bufsize, bool readonly) : 
    _allocator(nullptr), 
    _data(buf), 
    _cap(bufsize), 
    _pos(0), 
    _limit(bufsize), 
    _mark(-1),
    _order(system::native_byte_order()),
    m_readonly(readonly) {

    }
    
    byte_buffer::byte_buffer(int64_t capacity, tca::base_allocator* allocator) : 
    _allocator(allocator), 
    _data(nullptr), 
    _cap(0), 
    _pos(0), 
    _limit(0), 
    _mark(-1),
    _order(system::native_byte_order())  {
        if (capacity > 0) {
            char* data = (char*) allocator->allocate_align(capacity, alignof(char));
            if (data == nullptr)
                throw_except<out_of_memory_error>("Out of memory");
            _allocator  = allocator;
            _data       = data;
            _cap        = capacity;
            _limit      = _cap;
            _pos        = 0;
        }
    }
    
    byte_buffer::byte_buffer(const byte_buffer& buf) : byte_buffer(0) {
        assert(true);
    }
    
    byte_buffer::byte_buffer(byte_buffer&& buf) : 
    _allocator(buf._allocator), 
    _data(buf._data), 
    _cap(buf._cap), 
    _pos(buf._pos),
    _limit(buf._limit), 
    _mark(buf._mark),
    _order(buf._order),
    m_readonly(buf.m_readonly) {
        buf._allocator  = nullptr;
        buf._data       = nullptr;
        buf._cap        = 0;
        buf._pos        = 0;
        buf._limit      = 0;
        buf._mark       =-1;
        buf.m_readonly  = false;
    }
    
    byte_buffer& byte_buffer::operator= (const byte_buffer& buf) {
        assert(true);
        return *this;
    }
    
    byte_buffer& byte_buffer::operator= (byte_buffer&& buf) {
        if (&buf != this) {
            dispose();
            _allocator  = buf._allocator;
            _data       = buf._data;
            _cap        = buf._cap;
            _pos        = buf._pos;
            _limit      = buf._limit;
            _mark       = buf._mark;
            _order      = buf._order;
            m_readonly  = buf.m_readonly;
            buf._allocator  = nullptr;
            buf._data       = nullptr;
            buf._cap        = 0;
            buf._pos        = 0;
            buf._limit      = 0;
            buf._mark       =-1;
            buf.m_readonly  = false;
        }
        return *this;
    }
    
    void byte_buffer::dispose() {
        if (_allocator != nullptr) {
            if (_data != nullptr)
                _allocator->deallocate(_data, _cap);
        }
    }

    byte_buffer::~byte_buffer() {
        dispose();
    }

    /*static*/ byte_buffer byte_buffer::allocate(int64_t capacity, tca::base_allocator* allocator) {
        return byte_buffer(capacity, allocator);
    }

    int64_t byte_buffer::position() const {
        return _pos;
    }
    
    byte_buffer& byte_buffer::position(int64_t newpos) {
        assert(newpos <= _limit && newpos >= 0);
        if (_mark > newpos) _mark = -1;
        _pos = newpos;
        return *this;
    }

    int64_t byte_buffer::limit() const {
        return _limit;
    }
    
    byte_buffer& byte_buffer::limit(int64_t newlimit){
        assert(newlimit <= _cap && newlimit >= 0);
        _limit = newlimit;
        if (_pos > newlimit)  _pos  = newlimit;
        if (_mark > newlimit) _mark = -1;
        return *this;
    }

    int64_t byte_buffer::remaining() const {
        if (_pos > _limit)
            return 0;
        return _limit - _pos;
    }

    int64_t byte_buffer::capacity() const {
        return _cap;
    }

    byte_buffer& byte_buffer::clear() {
        _pos    = 0;
        _limit  = _cap;
        _mark   = -1;
        return *this;
    }
    
    byte_buffer& byte_buffer::compact() {
        char* src = _data + _pos;
        char* dst = _data;
        int64_t len = _limit - _pos;
        if (len > 0)
            std::memmove(dst, src, len);
        _pos    = len;
        _limit  = _cap;
        _mark   = -1;
        return *this;
    }
    
    byte_buffer& byte_buffer::flip() {
        _limit  = _pos;
        _pos    = 0;
        _mark   = -1;
        return *this;
    }

    byte_buffer& byte_buffer::mark() {
        _mark = _pos;
        return *this;
    }

    byte_order byte_buffer::order() const {
        return _order;
    }

    char* byte_buffer::data() {
        return _data;
    }
    
    const char* byte_buffer::data() const {
        return _data;
    }
    
    byte_buffer& byte_buffer::order(byte_order order) {
        _order = order;
        return *this;
    }

#define ______MAKE____SPECIALIZATION____ALLOCATORS_LIB_BYTE_BUFFER_IMPL(type_name__jstd_io_bytebuffer)                                                      \
                                                                                                                                                            \
    template<>                                                                                                                                              \
    byte_buffer& byte_buffer::put<type_name__jstd_io_bytebuffer>(type_name__jstd_io_bytebuffer v) {                                                         \
        check_write_or_except();                                                                                                                            \
        put<type_name__jstd_io_bytebuffer>(_pos, v);                                                                                                        \
        _pos += sizeof(v);                                                                                                                                  \
        return *this;                                                                                                                                       \
    }                                                                                                                                                       \
                                                                                                                                                            \
    template<>                                                                                                                                              \
    byte_buffer& byte_buffer::put<type_name__jstd_io_bytebuffer>(int64_t idx, type_name__jstd_io_bytebuffer v) {                                            \
        checkIndex(idx, sizeof(v));                                                                                                                         \
        check_write_or_except();                                                                                                                            \
        _data[idx] = static_cast<type_name__jstd_io_bytebuffer>(v);                                                                                         \
        return *this;                                                                                                                                       \
    }                                                                                                                                                       \
                                                                                                                                                            \
    template<>                                                                                                                                              \
    byte_buffer& byte_buffer::puts<type_name__jstd_io_bytebuffer>(const type_name__jstd_io_bytebuffer* arr, int64_t sz) {                                   \
        check_write_or_except();                                                                                                                            \
        puts<type_name__jstd_io_bytebuffer>(_pos, arr, sz);                                                                                                 \
        _pos += sizeof(type_name__jstd_io_bytebuffer) * sz;                                                                                                 \
        return *this;                                                                                                                                       \
    }                                                                                                                                                       \
                                                                                                                                                            \
    template<>                                                                                                                                              \
    byte_buffer& byte_buffer::puts<type_name__jstd_io_bytebuffer>(int64_t idx, const type_name__jstd_io_bytebuffer* arr, int64_t sz) {                      \
        check_write_or_except();                                                                                                                            \
        checkIndex(idx, sizeof(type_name__jstd_io_bytebuffer) * sz);                                                                                        \
        std::memcpy(_data + idx, reinterpret_cast<const char*>(arr), sizeof(type_name__jstd_io_bytebuffer) * sz);                                           \
        return *this;                                                                                                                                       \
    }                                                                                                                                                       \
                                                                                                                                                            \
    template<>                                                                                                                                              \
    type_name__jstd_io_bytebuffer byte_buffer::get<type_name__jstd_io_bytebuffer>() {                                                                       \
        type_name__jstd_io_bytebuffer tmp = get<type_name__jstd_io_bytebuffer>(_pos);                                                                       \
        _pos += sizeof(tmp);                                                                                                                                \
        return tmp;                                                                                                                                         \
    }                                                                                                                                                       \
                                                                                                                                                            \
    template<>                                                                                                                                              \
    type_name__jstd_io_bytebuffer byte_buffer::get<type_name__jstd_io_bytebuffer>(int64_t idx) const {                                                  \
        checkIndex(idx, sizeof(type_name__jstd_io_bytebuffer));                                                                                             \
        return static_cast<type_name__jstd_io_bytebuffer>(_data[idx]);                                                                                      \
    }                                                                                                                                                       \
                                                                                                                                                            \
    template<>                                                                                                                                              \
    byte_buffer& byte_buffer::gets(type_name__jstd_io_bytebuffer* arr, int64_t sz) {                                                                    \
        gets<type_name__jstd_io_bytebuffer>(_pos, arr, sz);                                                                                                 \
        _pos += sizeof(type_name__jstd_io_bytebuffer) * sz;                                                                                                 \
        return *this;                                                                                                                                       \
    }                                                                                                                                                       \
                                                                                                                                                            \
    template<>                                                                                                                                              \
    byte_buffer& byte_buffer::gets<type_name__jstd_io_bytebuffer>(int64_t idx, type_name__jstd_io_bytebuffer* arr, int64_t sz) {                    \
        checkIndex(idx, sizeof(type_name__jstd_io_bytebuffer) * sz);                                                                                        \
        std::memcpy(reinterpret_cast<char*>(arr), _data + idx, sizeof(type_name__jstd_io_bytebuffer) * sz);                                                 \
        return *this;                                                                                                                                       \
    }                                                                                                                                                       \

______MAKE____SPECIALIZATION____ALLOCATORS_LIB_BYTE_BUFFER_IMPL(char)
______MAKE____SPECIALIZATION____ALLOCATORS_LIB_BYTE_BUFFER_IMPL(signed char)
______MAKE____SPECIALIZATION____ALLOCATORS_LIB_BYTE_BUFFER_IMPL(unsigned char)

#undef ______MAKE____SPECIALIZATION____ALLOCATORS_LIB_BYTE_BUFFER_IMPL

    void byte_buffer::print() const {
        std::printf("[position=%zu, limit=%zu, capacity=%zu]\n", _pos, _limit, _cap);
    }

    int byte_buffer::to_string(char buf[], int bufsize) const {
        return std::snprintf(buf, bufsize, "[position=%zu, limit=%zu, capacity=%zu]\n", _pos, _limit, _cap);
    }
}