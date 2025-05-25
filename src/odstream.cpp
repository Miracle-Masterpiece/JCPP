#include <cpp/lang/io/odstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>

namespace jstd {

    odstream::odstream() : _out(nullptr) {

    }

    odstream::odstream(ostream* out) : _out(out) {
        if (out == nullptr)
            throw_except<null_pointer_exception>("stream is null!");
    }

    odstream::odstream(odstream&& stream) : _out(stream._out) {
        stream._out = nullptr;
    }
    
    odstream& odstream::operator= (odstream&& stream) {
        if (&stream != this) {
            if (_out != nullptr)
                close();
            _out        = stream._out;
            stream._out = nullptr;
        }
        return *this;
    }
    
    odstream::~odstream() {
        if (_out != nullptr) {
            try {
                close();
            } catch (const io_exception& e) {
                std::cout << e.cause() << "\n";
            }
        }
    }

    void odstream::write(const char* data, int64_t sz) {
#ifndef NDEBUG
        if (_out == nullptr)
            throw_except<io_exception>("Stream is null!");
#endif
        _out->write(data, sz);
    }
    
    void odstream::flush() {
#ifndef NDEBUG
        if (_out == nullptr)
            throw_except<io_exception>("Stream is null!");
#endif
        _out->flush();
    }
    
    void odstream::close() {
#ifndef NDEBUG
        if (_out == nullptr)
            throw_except<io_exception>("Stream is null!");
#endif
        try {
            _out->close();
            _out = nullptr;
        } catch (...) {
            _out = nullptr;
            throw;
        }
    }
}