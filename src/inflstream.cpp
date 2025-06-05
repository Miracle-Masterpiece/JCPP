#include <cpp/lang/io/inflstream.hpp>
#include <iostream>

namespace jstd {

    inflstream::inflstream() : m_buffer(), m_in(nullptr), m_inf() {

    }

    inflstream::inflstream(istream* in, tca::base_allocator* allocator, int64_t buf_size) :
    m_buffer(buf_size, allocator), m_in(in), m_inf() {
#ifndef NDEBUG
        if (in == nullptr)
            throw_except<null_pointer_exception>("Input buffer must be != null!");
#endif

    }

    int64_t inflstream::read(char buf[], int64_t sz) {
#ifndef NDEBUG
        if (m_in == nullptr)
            throw_except<io_exception>("Stream is null!");
#endif//NDEBUG
        if (sz == 0)
            return 0;

        int64_t len = 0;
        do {
            if (m_inf.is_finished())
                return -1;
            if (m_inf.needs_input())
                fill();
        } while ((len = m_inf.inflate(buf, sz)) == 0);
        
        return len;
    }

    void inflstream::fill() {
        int64_t readed = m_in->read(m_buffer.data(), m_buffer.length);
        if (readed == -1)
            throw_except<eof_exception>("Unexpected end of ZLIB input stream");
        m_inf.set_input(m_buffer.data(), readed);
    }

    int64_t inflstream::available() const {
        if (m_inf.is_finished())
            return 0;
        return 1;
    }

    inflstream::~inflstream() {
        try {
            if (m_in != nullptr)
                close();
        } catch (const throwable& t) {
            std::cout << t.cause() << '\n';
        }
    }

    void inflstream::close() {
#ifndef NDEBUG
        if (m_in == nullptr)
            throw_except<io_exception>("Stream already closed!");
#endif//NDEBUG
        try {
            m_in->close();
            m_in = nullptr;
        } catch (const io_exception& e) {
            m_in = nullptr;
            throw e;
        }
    }

}