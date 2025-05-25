#include <cpp/lang/io/ifstream.hpp>
#include <cpp/lang/io/filesystem.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/system.hpp>
#include <errno.h>
#include <cstring>
#include <iostream>

namespace jstd {

    ifstream::ifstream() : istream(), _handle(nullptr), _available(0) {

    }

    ifstream::ifstream(const char* path, int path_length) : ifstream(file(path, path_length)) {

    }
    
    ifstream::ifstream(const file& file) : _handle(nullptr), _available(0) {
        //функция filesystem::open уже кидает нужные исключения.
        _handle = filesystem::open(file.str_path(), "rb");
        try {
            _available = filesystem::length(file.str_path());
        } catch (const io_exception& except) {
            try {
                filesystem::close(_handle);
            } catch (const throwable& dontCare) {
                std::cout << dontCare.cause() << "\n";
            }
            throw except;
        }
    }
    
    ifstream::ifstream(ifstream&& stream) : istream(), _handle(stream._handle), _available(stream._available) {
        stream._handle      = nullptr;
        stream._available   = 0;
    }
    
    ifstream& ifstream::operator= (ifstream&& stream) {
        if (&stream != this) {
            if (_handle != nullptr)
                close();
            _handle     = stream._handle;
            _available  = stream._available;
            stream._handle      = nullptr;
            stream._available   = 0;
        }
        return *this;
    }
    
    void ifstream::close() {
        if (_handle == nullptr)
            throw_except<io_exception>("Stream already closed!");
        if (_handle != nullptr) {
            try {
                filesystem::close(_handle);
            } catch (const io_exception& e) {
                _handle     = nullptr;
                _available  = 0;
                throw e;
            }
            _handle     = nullptr;
            _available  = 0;
        }
    }
    
    ifstream::~ifstream() {
        try {
            if (_handle != nullptr)
                close();
        } catch (const io_exception& dont_care) {
            std::cout << dont_care.cause() << "\n";
        }
    }
    
    int ifstream::read() {
        return istream::read();
    }
    
    int64_t ifstream::read(char buf[], int64_t sz) {
#ifndef NDEBUG
        if (_handle == nullptr)
            throw_except<io_exception>("File stream is null!");
#endif
        int64_t readed = fread(buf, 1, sz, _handle);   
        if (readed == 0) {
            if (feof(_handle))
                return -1;
            if (ferror(_handle))
                throw_except<io_exception>(strerror(errno));
        }
        _available -= readed;
        return readed;
    }
    
    int64_t ifstream::skip(int64_t n) {
#ifndef NDEBUG
        if (_handle == nullptr)
            throw_except<io_exception>("File stream is null!");
#endif
        return istream::skip(n);
    }
    
    int64_t ifstream::available() const {
#ifndef NDEBUG
        if (_handle == nullptr)
            throw_except<io_exception>("File stream is null!");
#endif
        return _available;
    }

}