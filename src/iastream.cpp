#include <cpp/lang/io/iastream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>

namespace jstd {


    iastream::iastream() : _buffer(nullptr), _capacity(0), _offset(0) {

    }

    iastream::iastream(const char* buffer, int64_t capacity) : _buffer(buffer), _capacity(capacity), _offset(0) {
        if (buffer == nullptr)
            throw_except<null_pointer_exception>("buffer is null");
    }
    
    iastream::iastream(iastream&& stream) : _buffer(stream._buffer), _capacity(stream._capacity), _offset(stream._offset) {
        stream._buffer      = nullptr;
        stream._capacity    = 0;
        stream._offset      = 0;
    }
    
    iastream& iastream::operator= (iastream&& stream) {
        if (&stream != this) {
            if (_buffer != nullptr)
                close();
            _buffer     = stream._buffer;
            _capacity   = stream._capacity;
            _offset     = stream._offset;

            stream._buffer      = nullptr;
            stream._capacity    = 0;
            stream._offset      = 0;
        }
        return *this;
    }
    
    iastream::~iastream() {
        if (_buffer != nullptr) {
            try  {
                close();
            } catch (const io_exception& e) {
                std::cout << e.cause() << "\n";
            }
            _buffer = nullptr;
        }
    }
    
    int iastream::read() {
        return istream::read();
    }
    
    int64_t iastream::read(char buf[], int64_t sz) {        
#ifndef NDEBUG
        if (_buffer == nullptr)
            throw_except<io_exception>("Stream is null");
#endif//NDEBUG
        sz = std::min(available(), sz);
        if (sz == 0)
            return -1;

        std::memcpy(buf, _buffer + _offset, sz);
        _offset += sz;
        
        return sz;
    }
    
    int64_t iastream::available() const {
#ifndef NDEBUG
        if (_buffer == nullptr)
            throw_except<io_exception>("Stream is null");
#endif//NDEBUG
        return _capacity - _offset;
    }
    
    void iastream::close() {
        if (_buffer != nullptr) {
            _buffer = nullptr;
        } else {
            throw_except<io_exception>("Stream already closed");
        }
    }
    
    void iastream::reset() {
        _offset = 0;
    }
}