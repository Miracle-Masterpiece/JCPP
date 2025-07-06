#include <internal/io/file_descriptor.hpp>

#include <allocators/inline_linear_allocator.hpp>
#include <cpp/lang/utils/cond_compile.hpp>
#include <cpp/lang/ustring.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/io/constants.hpp>
#include <errno.h>
#include <cstring>

#if defined(JSTD_OS_WINDOWS)
#   include <windows.h>
#   include <io.h>
#   include <sys/fcntl.h>
#   include <sys/types.h>
#   define JSTD_WIN_CODE(code) code
#   define JSTD_POSIX_CODE(code)
#elif defined(JSTD_OS_UNIX) || defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC)
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#   include <unistd.h>
#   define JSTD_WIN_CODE(code)    
#   define JSTD_POSIX_CODE(code) code
#endif

namespace jstd
{
namespace internal
{
    const file_descriptor file_descriptor::NULL_FD(-1);

    file_descriptor::file_descriptor(int fd) : fd(fd) {

    }

    file_descriptor::file_descriptor() : file_descriptor(NULL_FD.fd) {

    }

    file_descriptor::file_descriptor(file_descriptor&& fdesc) : fd(fdesc.fd) {
        fdesc.fd = NULL_FD.fd;
    }
    
    file_descriptor& file_descriptor::operator=(file_descriptor&& fdesc) {
        if (&fdesc != this) {
            cleanup();
            fd          = fdesc.fd;
            fdesc.fd    = NULL_FD.fd;
        }
        return *this;
    }
    
    file_descriptor::file_descriptor(const char* u8path, int oflags, int pmode) {
        JSTD_WIN_CODE(
            tca::inline_linear_allocator<io::constants::MAX_LENGTH_PATH * sizeof(wchar_t)> allocator;
            u16string wname = jstd::make_utf16(u8path, &allocator, jstd::byte_order::LE);        
            fd = _wopen(reinterpret_cast<const wchar_t*>(wname.c_string()), oflags, pmode);
        );
        JSTD_POSIX_CODE(
            fd = open(u8path, oflags, pmode);    
        );
        if (fd == NULL_FD.fd) {
            if (errno == EACCES)
                throw_except<sequrity_exception>(std::strerror(errno));
            throw_except<io_exception>(std::strerror(errno));
        }
    }
    
    void file_descriptor::cleanup() {
        if (*this != NULL_FD) {
            JSTD_WIN_CODE(
                _close(fd);
            );
            JSTD_POSIX_CODE(
                close(fd);
            );
        }
    }

    file_descriptor::~file_descriptor() {
        cleanup();
    }
    
    int file_descriptor::get_fd() const {
        return fd;
    }

    bool file_descriptor::operator==(const file_descriptor& fdesc) const {
        return fd == fdesc.fd;
    }

    bool file_descriptor::operator!=(const file_descriptor& fdesc) const {
        return fd != fdesc.fd;
    }
}

}