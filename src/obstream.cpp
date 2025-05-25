#include <cpp/lang/io/obstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>

namespace jstd {

    obstream::obstream() : _allocator(nullptr), _buffer(nullptr), _capacity(0), _offset(0) {

    }

    obstream::obstream(ostream* stream, tca::base_allocator* allocator, int64_t buf_size) : obstream() {
        if (stream == nullptr)
            throw_except<null_pointer_exception>("stream is null!");
        if (allocator == nullptr)
            throw_except<null_pointer_exception>("allocator is null!");
        char* data  = (char*) allocator->allocate(buf_size);
        _allocator  = allocator;
        _buffer     = data;
        _capacity   = buf_size;
        _offset     = 0;
        _out        = stream;
    }
    
    obstream::obstream(ostream* stream, char* buffer, int64_t buf_size) : obstream() {
        if (stream == nullptr)
            throw_except<null_pointer_exception>("stream is null!");
        if (buffer == nullptr)
            throw_except<null_pointer_exception>("buffer is null!");
        _buffer     = buffer;
        _capacity   = buf_size;
        _offset     = 0;
        _out        = stream;
    }

    obstream::obstream(obstream&& stream) : 
    _allocator(stream._allocator), _buffer(stream._buffer), _capacity(stream._capacity), _offset(stream._offset), _out(stream._out) {
        stream._allocator   = nullptr;
        stream._buffer      = nullptr;
        stream._capacity    = 0;
        stream._offset      = 0;
        stream._out         = nullptr;
    }

    obstream& obstream::operator= (obstream&& stream) {
        if (&stream != this) {
            if (_out != nullptr)
                close();
            _allocator  = stream._allocator;
            _buffer     = stream._buffer;
            _capacity   = stream._capacity;
            _offset     = stream._offset;
            _out        = stream._out;

            stream._allocator   = nullptr;
            stream._buffer      = nullptr;
            stream._capacity    = 0;
            stream._offset      = 0;
            stream._out         = nullptr;
        }
        return *this;
    }

    void obstream::free() {
        if (_allocator != nullptr) {
            _allocator->deallocate(_buffer, _capacity);
            _allocator = nullptr;
        }
    }

    obstream::~obstream() {
        if (_out != nullptr) {
            try {
                close();
            } catch (const io_exception& e) {
                std::cout << e.cause() << "\n";
            }
        }
        if (_allocator != nullptr) {
            _allocator->deallocate(_buffer, _capacity);
            _allocator = nullptr;
        }
    }

    void obstream::write(char c) {
        ostream::write(c);
    }
    
    void obstream::write(const char* data, int64_t sz) {
#ifndef NDEBUG
        if (_out == nullptr)
            throw_except<io_exception>("Stream is null!");
#endif
        if (sz > _capacity - _offset) {
            _out->write(_buffer, _offset);
            _offset = 0;
            if (sz < _capacity) {
                std::memcpy(_buffer, data, sz);
                _offset += sz;
            } else {
                _out->write(data, sz);
            }
        } else {
            std::memcpy(_buffer + _offset, data, sz);
            _offset += sz;
        }
    }
    
    void obstream::flush() {
#ifndef NDEBUG
        if (_out == nullptr)
            throw_except<io_exception>("Stream is null!");
#endif
        if (_offset > 0) {
            _out->write(_buffer, _offset);
            _offset = 0;
        }
    }
    
    void obstream::close() {
        if (_out != nullptr) {
            try {
                flush();
                _out->close();
            } catch (io_exception& e) {
                _out = nullptr;
                free();
                throw e;
            }
            _out = nullptr;
            free();
        } else {
            throw_except<io_exception>("Stream already closed!");
        }
    }
    
}