#include <cpp/lang/io/ibstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>

namespace jstd {

    ibstream::ibstream() : _allocator(nullptr), _buffer(nullptr), _capacity(0), _offset(0), _limit(0), _in(nullptr) {

    }

    ibstream::ibstream(istream* stream, tca::base_allocator* allocator, int64_t buf_size) : ibstream() {
        char* data = (char*) allocator->allocate(buf_size);
        _allocator  = allocator;
        _buffer     = data;
        _capacity   = buf_size;
        _offset     = 0;
        _limit      = 0;
        _in         = stream;
    }
    
    ibstream::ibstream(istream* stream, char* buf, int64_t buf_size) : ibstream() {
        _buffer     = buf;
        _capacity   = buf_size;
        _offset     = 0;
        _limit      = 0;
        _in         = stream;
    }
    
    ibstream::ibstream(ibstream&& stream) : 
    _allocator(stream._allocator), 
    _buffer(stream._buffer),
    _capacity(stream._capacity),
    _offset(stream._offset),
    _limit(stream._limit),
    _in(stream._in) {
        stream._allocator   = nullptr;
        stream._buffer      = nullptr;
        stream._capacity    = 0;
        stream._offset      = 0;
        stream._limit       = 0;
        stream._in          = nullptr;
    }
    
    ibstream& ibstream::operator= (ibstream&& stream) {
        if (&stream != this) {
            if (_in != nullptr)
                close();
            _allocator   = stream._allocator;
            _buffer      = stream._buffer;
            _capacity    = stream._capacity;
            _offset      = stream._offset;
            _limit       = stream._limit;
            _in          = stream._in;
            
            stream._allocator   = nullptr;
            stream._buffer      = nullptr;
            stream._capacity    = 0;
            stream._offset      = 0;
            stream._limit       = 0;
            stream._in          = nullptr;
        }
        return *this;
    }
    
    int ibstream::read() {
        return istream::read();
    }
    
    int64_t ibstream::read(char buf[], int64_t sz) {
#ifndef NDEBUG
        if (_in == nullptr)
            throw_except<io_exception>("Stream is null");
#endif
        if (_limit == -1)
            return -1;
        if (_limit - _offset < sz) {
            int64_t remaining = _limit - _offset;
            std::memcpy(buf, _buffer + _offset, remaining);
            _offset = 0;
            int64_t readed = _in->read(buf + remaining, sz - remaining);
            _offset = 0;
            _limit  = _in->read(_buffer, _capacity);
            readed = readed > 0 ? readed : 0;

            if (readed + remaining == 0)
                return -1;
            else
                return readed + remaining;
        } else {
            std::memcpy(buf, _buffer + _offset, sz);
            _offset += sz;
            return sz;
        }
    }
    
    int64_t ibstream::skip(int64_t n) {
#ifndef NDEBUG
        if (_in == nullptr)
            throw_except<io_exception>("Stream is null");
#endif
        return istream::skip(n);
    }
    
    int64_t ibstream::available() const {
        const int64_t available_in_buffer = _limit - _offset;
        return available_in_buffer + _in->available();
    }
    
    void ibstream::free() {
        if (_allocator != nullptr) {
            _allocator->deallocate(_buffer, _capacity);
            _allocator  = nullptr;
        }
    }

    ibstream::~ibstream() {
        if (_in != nullptr) {
            try {
                close();
            } catch (const io_exception& e) {
                std::cout << e.cause() << std::endl;
            }    
        }
    }

    void ibstream::close() {
        if (_in != nullptr) {
            try {
                _in->close();
            } catch (const io_exception& e) {
                _in = nullptr;
                free();
                throw e;    
            }
            _in = nullptr;
            free();
        } else {
            throw_except<io_exception>("Stream already closed!");
        }   
    }    
}