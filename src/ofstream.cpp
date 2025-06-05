#include <cpp/lang/io/ofstream.hpp>
#include <cpp/lang/io/filesystem.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/system.hpp>
#include <errno.h>
#include <cstring>
#include <iostream>

namespace jstd {

    ofstream::ofstream() : _handle(nullptr) {

    }

    ofstream::ofstream(const char* path, bool append, int path_length) : ofstream(file(path, path_length), append) {
        
    }
    
    ofstream::ofstream(const file& f, bool append) : ofstream() {    
        _handle = filesystem::open(f.str_path(), append ? "ab" : "wb");
    }

    ofstream::ofstream(ofstream&& stream) : _handle(stream._handle) {
        stream._handle = nullptr;
    }
    
    ofstream& ofstream::operator= (ofstream&& stream) {
        if (&stream != this) {
            if (_handle != nullptr)
                close();
            _handle = stream._handle;
            stream._handle = nullptr;
        }
        return *this;
    }
    
    ofstream::~ofstream() {
        try {
            if (_handle != nullptr)
                close();
        } catch (const io_exception& e) {
            std::cout << e.cause() << std::endl;
        }
    }
    
    void ofstream::write(char c) {
        return ostream::write(c);
    }
    
    void ofstream::write(const char* data, int64_t sz) {
#ifndef NDEBUG
        if (_handle == nullptr)
            throw_except<io_exception>("File stream is null!");
#endif
        int64_t writed = fwrite(data, 1, sz, _handle);
        if (writed != sz)
            throw_except<io_exception>(strerror(errno));
    }
    
    void ofstream::flush() {
        filesystem::flush(_handle);
    }
    
    void ofstream::close() {
        if (_handle != nullptr) {
            try {
                flush();
                filesystem::close(_handle);
            } catch (const io_exception& e) {
                _handle     = nullptr;
                throw e;
            }
            _handle     = nullptr;
        } else {
            throw_except<io_exception>("Stream already closed!");
        }
    }    
}