#include <cpp/lang/io/idstream.hpp>
#include <iostream>

namespace jstd {

    idstream::idstream() : _in(nullptr) {
        
    }

    idstream::idstream(istream* in) : _in(in) {
        if (in == nullptr)
            throw_except<null_pointer_exception>("in is null");
    }
    
    idstream::idstream(idstream&& stream) : _in(stream._in) {
        stream._in = nullptr;
    }

    idstream& idstream::operator= (idstream&& stream) {
        if (&stream != this) {
            if (_in != nullptr)
                close();
            _in         = stream._in;
            stream._in  = nullptr;
        }
        return *this;
    }

    idstream::~idstream() {
        if (_in != nullptr) {
            try {
                _in->close();
            } catch (const io_exception& e) {
                std::cout << e.cause() << '\n';
            }
        }
    }
    
    int64_t idstream::read(char buf[], int64_t sz) {
#ifndef NDEBUG
        if (_in == nullptr)
            throw_except<io_exception>("Stream is null!");
#endif
        return _in->read(buf, sz);
    }

    int64_t idstream::available() const {
#ifndef NDEBUG
        if (_in == nullptr)
            throw_except<io_exception>("Stream is null!");
#endif
        return _in->available();
    }

    void idstream::close() {
#ifndef NDEBUG
        if (_in == nullptr)
            throw_except<io_exception>("Stream is null!");
#endif
        try {
            _in->close();
            _in = nullptr;
        } catch (...) {
            _in = nullptr;
            throw;
        }
    }
}