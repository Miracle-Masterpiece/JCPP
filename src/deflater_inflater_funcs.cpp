#include <zlib.h>
#include <internal/deflater_inflater_funcs.h>

namespace jstd {
namespace zip {
namespace internal {

    const char* zlib_errorcode_to_string(int err) {
        switch(err) {
            case Z_OK:
                return "Z_OK";
            case Z_STREAM_END:
                return "Z_STREAM_END";
            case Z_NEED_DICT:
                return "Z_NEED_DICT";
            case Z_ERRNO:
                return "Z_ERRNO";
            case Z_STREAM_ERROR:
                return "Z_STREAM_ERROR";
            case Z_DATA_ERROR:
                return "Z_DATA_ERROR";
            case Z_MEM_ERROR:
                return "Z_MEM_ERROR";
            case Z_BUF_ERROR:
                return "Z_BUF_ERROR";
            case Z_VERSION_ERROR:
                return "Z_VERSION_ERROR";
            default:
                return "UNKNOW_ERROR";
        }
    }

}//namespace internal
}//namespace zip
}//namespace jstd