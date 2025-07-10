#include <cpp/lang/io/deflstream.hpp>
#include <iostream>

namespace jstd {

    deflstream::deflstream() : m_buffer(), m_out(nullptr), m_def() {

    }

    deflstream::deflstream(ostream* out, tca::base_allocator* allocator, int64_t buf_size) :
    m_buffer(buf_size, allocator), m_out(out), m_def() {
        
    }

    deflstream::deflstream(deflstream&& def) : 
    m_buffer(std::move(def.m_buffer)),
    m_out(def.m_out),
    m_def(std::move(def.m_def)) {
        def.m_out       = nullptr;
    }
    
    deflstream& deflstream::operator= (deflstream&& def) {
        if (&def != this) {
            if (m_out != nullptr)
                close();
            m_buffer    = std::move(def.m_buffer);
            m_out       = def.m_out;
            m_def       = std::move(def.m_def);
            def.m_out       = nullptr;
        }
        return *this;
    }

    deflstream::deflstream(ostream* out, char* buffer, int64_t buf_size) :
    m_buffer(), m_out(out), m_def() {
        JSTD_DEBUG_CODE(
            if (buffer == nullptr)
                throw_except<null_pointer_exception>("Buffer must be != null");
            if (buf_size < 0)
                throw_except<illegal_argument_exception>("Invalid buffer size: %li", (long int) buf_size);
        );
        m_def.set_input(buffer, buf_size);
    }

    void deflstream::write(const char* data, int64_t sz) {
        JSTD_DEBUG_CODE(
            if (m_out == nullptr)
                throw_except<io_exception>("Stream is null!");
            if (m_def.is_finished())
                throw_except<io_exception>("Write beyond end of stream");
            if (sz < 0)
                throw_except<illegal_argument_exception>("Invalid buffer size: %li", (long int) sz);
        );

        if (sz == 0)
            return;

        if (!m_def.is_finished()) {
            m_def.set_input(data, sz);
            while (!m_def.needs_input()) {
                std::size_t out_size = m_def.deflate(m_buffer.data(), m_buffer.length);
                if (out_size > 0)
                    m_out->write(m_buffer.data(), out_size);
            }
        }
    }

    void deflstream::flush() {
#ifndef NDEBUG
        if (m_out == nullptr)
            throw_except<io_exception>("Stream is null!");
#endif//NDEBUG
        m_out->flush();
    }

    void deflstream::finish() {
#ifndef NDEBUG
        if (m_out == nullptr)
            throw_except<io_exception>("Stream is null!");
#endif//NDEBUG
        if (!m_def.is_finished()) {
            m_def.finish();
            while (!m_def.is_finished()) {
                std::size_t sz = m_def.deflate(m_buffer.data(), m_buffer.length);
                m_out->write(m_buffer.data(), sz);
            }
        }
    }

    void deflstream::close() {
        if (m_out == nullptr)
            return;
        try {
            try {
                finish();
            } catch (const io_exception& finish_except) {
                std::cout << finish_except.cause() << '\n';
            }
            m_out->close();
            m_out       = nullptr;
        } catch (const io_exception& e) {
            m_out       = nullptr;
            throw e;
        }
    }

    deflstream::~deflstream() {
 
    }

}