#include <cpp/lang/zip/inflater.hpp>
#include <cpp/lang/exceptions.hpp>
#include <internal/deflater_inflater_funcs.h>
#include <utility>
#include <cstring>

namespace jstd {


    inflater::inflater() 
        : finished(false) {
        std::memset(&strm, 0, sizeof(strm));
        inflateInit(&strm);
    }

    inflater::inflater(inflater&& i) : strm(std::move(i.strm)), finished(i.finished) {
        std::memset(&i.strm, 0, sizeof(i.strm));
    }

    inflater& inflater::operator= (inflater&& i) {
        if (&i != this) {
            inflateEnd(&strm);
            strm        = std::move(i.strm);
            finished    = i.finished;
            std::memset(&i.strm, 0, sizeof(i.strm));
        }
        return *this;
    }

    inflater::~inflater() {
        inflateEnd(&strm);
    }

    void inflater::set_input(const void* data, size_t size) {
        strm.next_in  = static_cast<Bytef*>(const_cast<void*>(data));
        strm.avail_in = static_cast<uInt>(size);
    }

    size_t inflater::inflate(void* outBuffer, size_t outBufferSize, bool finish) {
        if (finished) 
            return 0;

        strm.next_out   = reinterpret_cast<Bytef*>(outBuffer);
        strm.avail_out  = static_cast<uInt>(outBufferSize);

        int flush = finish ? Z_FINISH : Z_NO_FLUSH;

        int ret = ::inflate(&strm, flush);

        if (ret == Z_STREAM_END) {
            finished = true;
        } else if (ret != Z_OK && ret != Z_BUF_ERROR) {
            throw_except<illegal_state_exception>("inflate error: %s", zip::internal::zlib_errorcode_to_string(ret));
        }

        return outBufferSize - strm.avail_out;
    }

    bool inflater::needs_input() const {
        return strm.avail_in == 0;
    }

    bool inflater::is_finished() const {
        return finished;
    }

    void inflater::reset() {
        inflateReset(&strm);
        finished = false;
    }

    void inflater::set_dictionary(const void* dict, size_t dictLength) {
        inflateSetDictionary(&strm, reinterpret_cast<const Bytef*>(dict), static_cast<uInt>(dictLength));
    }
}