#include <cpp/lang/io/oastream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>

namespace jstd {

    oastream::oastream() : _allocator(nullptr), _buffer(nullptr), _capacity(0), _offset(0) {

    }
    
    oastream::oastream(tca::base_allocator* allocator, int64_t init_buf_size) : _allocator(allocator), _buffer(nullptr), _capacity(init_buf_size), _offset(0) {

    }

    oastream::oastream(char* buf, int64_t capacity) : _allocator(nullptr), _buffer(buf), _capacity(capacity), _offset(0) {

    }

    oastream::oastream(oastream&& stream) : 
    _allocator(stream._allocator), 
    _buffer(stream._buffer), 
    _capacity(stream._capacity), 
    _offset(stream._offset) {
        stream._allocator   = nullptr;
        stream._buffer      = nullptr;
        stream._capacity    = 0;
        stream._offset      = 0;
    }

    oastream& oastream::operator= (oastream&& stream) {
        if (&stream != this) {
            close();
            _allocator   = stream._allocator;
            _buffer      = stream._buffer;
            _capacity    = stream._capacity;
            _offset      = stream._offset;
            
            stream._allocator   = nullptr;
            stream._buffer      = nullptr;
            stream._capacity    = 0;
            stream._offset      = 0;
        }
        return *this;
    }


    void oastream::write(char c) {
        write(&c, 1);
    }

    void oastream::resize(int64_t sz) {
        int64_t s = _capacity == 0 ? INIT_BUF_SIZE : _capacity;
        
        //Вероятно - это один из самых худших алгоритмов для поиска размера не меньше, чем нужно. Но мне в данный момент всё-равно.
        while (sz + _offset > s) 
            s <<= 1;

        char* new_buffer = (char*) _allocator->allocate(s);
        if (new_buffer == nullptr)
            throw_except<null_pointer_exception>("Out of memory!");
        
        std::memcpy(new_buffer, _buffer, _offset);
        
        _allocator->deallocate(_buffer, _capacity);
        _buffer     = new_buffer;
        _capacity   = s;
    }

    void oastream::write(const char* data, int64_t sz) {
#ifndef NDEBUG
        if (_allocator == nullptr && _buffer == nullptr)
            throw_except<io_exception>("Stream is null");
#endif//NDEBUG

        if ((_offset + sz > _capacity) || (_buffer == nullptr)) {
            if (_allocator == nullptr)
                throw_except<overflow_exception>("oastream buffer is owerflow!");
            resize(sz);
        }
        
        std::memcpy(_buffer + _offset, data, sz);
        _offset += sz;
    }

    void oastream::flush() {
        //Это чёртово дерьмо тут только потому-что, попытка сбрасывание нулевого ресурса - логическая ошибка программиста!
#ifndef NDEBUG
        if (_allocator == nullptr && _buffer == nullptr)
            throw_except<io_exception>("Stream is null");
#endif//NDEBUG
    }

    void oastream::close() {
#ifndef NDEBUG
    if (_buffer == nullptr && _allocator == nullptr)
        throw_except<io_exception>("Stream already closed!");
#endif//NDEBUG
        if (_allocator != nullptr) {
            _allocator->deallocate(_buffer, _capacity);
            _allocator = nullptr;
        }
        _buffer = nullptr;
    }

    oastream::~oastream() {
        if (_buffer != nullptr || _allocator != nullptr)
            close();
    }

    const char* oastream::data() const {
        return _buffer;
    }
    
    int64_t oastream::offset() const {
        return _offset;
    }
}