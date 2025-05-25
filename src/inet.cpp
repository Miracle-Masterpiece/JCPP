#include <cpp/lang/net/inet.hpp>
#include <cpp/lang/exceptions.hpp>

#if defined(_WIN32)
#include <winsock2.h>
#include <internal/bsd_socket_errors.hpp>
#define __VERSION22 MAKEWORD(2, 2)
#define __VERSION20 MAKEWORD(2, 0)
#endif//_WIN32


namespace jstd {

    void init_inet() {
#if defined(_WIN32)
        WSAData data;
        int err = WSAStartup(__VERSION22, &data);
        if (err != 0)
            throw_except<socket_exception>("Windows socket initialize error: %s", bsd_socket::socket_error_string());
#endif
    }
    
    void close_inet() {
#if defined(_WIN32)
        WSACleanup();
#endif
    }
}