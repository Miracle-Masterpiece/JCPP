#include <cpp/lang/zip/deflater.hpp>
#include <internal/deflater_inflater_funcs.h>
#include <cpp/lang/exceptions.hpp>
#include <cstring>
#include <utility>

namespace jstd {

    deflater::deflater(int compressionLevel) : 
    m_strm(),
    m_finished(false),
    m_finish(false) {
        std::memset(&m_strm, 0, sizeof(m_strm));
        deflateInit(&m_strm, compressionLevel);
    }

    deflater::deflater(deflater&& d) : m_strm(std::move(d.m_strm)), m_finished(d.m_finished), m_finish(d.m_finish) {
        
    }

    deflater& deflater::operator= (deflater&& d) {
        if (&d != this) {
            deflateEnd(&m_strm);
            m_strm      = std::move(d.m_strm);       
            m_finished  = d.m_finished;
            m_finish    = d.m_finish;
        }
        return *this;
    }

    deflater::~deflater() {
        deflateEnd(&m_strm);
    }

    void deflater::set_input(const void* data, std::size_t size) {
        m_strm.next_in    = static_cast<Bytef*>(const_cast<void*>(data));
        m_strm.avail_in   = static_cast<uInt>(size);
    }

    std::size_t deflater::deflate(void* out_buffer, std::size_t out_buffer_size) {
        return deflate(out_buffer, out_buffer_size, NO_FLUSH);
    }

    std::size_t deflater::deflate(void* out_buffer, std::size_t out_buffer_size, int flush_type) {
        if (m_finished) 
            return 0;
#ifndef NDEBUG
        if (flush_type != NO_FLUSH && flush_type != FULL_FLUSH && flush_type != SYNC_FLUSH)
            throw_except<illegal_argument_exception>("invalid flush type %i", flush_type);
#endif//NDEBUG

        m_strm.next_out     = reinterpret_cast<Bytef*>(out_buffer);
        m_strm.avail_out    = static_cast<uInt>(out_buffer_size);

        if (m_finish)
            flush_type = FINISH;
        
        int ret     = ::deflate(&m_strm, flush_type);
        
        if (ret == Z_STREAM_END) {
            m_finished = true;
        } else if (ret != Z_OK && ret != Z_BUF_ERROR) {
            throw_except<illegal_state_exception>("deflate error: %s", zip::internal::zlib_errorcode_to_string(ret));
        }

        return out_buffer_size - m_strm.avail_out;
    }

    bool deflater::needs_input() const {
        return m_strm.avail_in == 0;
    }

    bool deflater::is_finished() const {
        return m_finished;
    }

    void deflater::reset() {
        deflateReset(&m_strm);
        m_finished  = false;
        m_finish    = false;
    }

    void deflater::finish() {
        m_finish = true;
    }

    void deflater::set_dictionary(const void* dict, size_t dictLength) {
        deflateSetDictionary(&m_strm, reinterpret_cast<const Bytef*>(dict), static_cast<uInt>(dictLength));
    }

}