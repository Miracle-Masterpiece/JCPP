#include <internal/bsd_socket.hpp>
#include <iostream>

#if defined(__linux__) || defined(__APPLE__)
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#elif _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else 
#error Unsupported platform!
#endif

#if defined(__WIN32)
#define _____WIN_CODE____(___code) ___code 
#define ___null_socket___ -1 
#define SHUT_RD SD_RECEIVE 
#define SHUT_WR SD_SEND
#else
#define _____WIN_CODE____(___code)
#endif
    
#if defined(__linux__) || defined(__APPLE__)
#define _____UNIX_CODE____(___code) ___code 
#define ___null_socket___ -1
#else
#define _____UNIX_CODE____(___code) 
#define ___null_socket___ -1
#endif

#define __WIN_KEEPALIVE_TRY_COUNT__ 10

#define __sock_error_str__ socket_error_string()

namespace jstd {
namespace bsd_socket {

    void ms_to_timeval(struct timeval* tv, int64_t millisec) {
        std::cout << "in ms: " << millisec << std::endl;
        tv->tv_sec   =  millisec / 1000;             
        tv->tv_usec  = (millisec % 1000) * 1000;
    }   

    int64_t timeval_to_ms(struct timeval* tv) {
        int64_t result = 0;
        result += tv->tv_sec * 1000;
        result += tv->tv_usec / 1000;
        return result;
    }

    const char* socket_error_string() {
        _____WIN_CODE____(
            int err = WSAGetLastError();
           switch(err) {
            case WSA_INVALID_HANDLE:
                return "Specified event object handle is invalid";
            case WSA_NOT_ENOUGH_MEMORY:
                return "Insufficient memory available";
            case WSA_INVALID_PARAMETER:
                return "One or more parameters are invalid";
            case WSA_OPERATION_ABORTED:
                return "Overlapped operation aborted";
            case WSA_IO_INCOMPLETE:
                return "Overlapped I/O event object not in signaled state";
            case WSA_IO_PENDING:
                return "Overlapped operations will complete later";
            case WSAEINTR:
                return "Interrupted function call";
            case WSAEBADF:
                return "File handle is not valid";
            case WSAEACCES:
                return "Permission denied";
            case WSAEFAULT:
                return "Bad address";
            case WSAEINVAL:
                return "Invalid argument";
            case WSAEMFILE:
                return "Too many open files";
            case WSAEWOULDBLOCK:
                return "Resource temporarily unavailable";
            case WSAEINPROGRESS:
                return "Operation now in progress";
            case WSAEALREADY:
                return "Operation already in progress";
            case WSAENOTSOCK:
                return "Socket operation on nonsocket";
            case WSAEDESTADDRREQ:
                return "Destination address required";
            case WSAEMSGSIZE:
                return "Message too long";
            case WSAEPROTOTYPE:
                return "Protocol wrong type for socket";
            case WSAENOPROTOOPT:
                return "Bad protocol option";
            case WSAEPROTONOSUPPORT:
                return "Protocol not supported";
            case WSAESOCKTNOSUPPORT:
                return "Socket type not supported";
            case WSAEOPNOTSUPP:
                return "Operation not supported";
            case WSAEPFNOSUPPORT:
                return "Protocol family not supported";
            case WSAEAFNOSUPPORT:
                return "Address family not supported by protocol family";
            case WSAEADDRINUSE:
                return "Address already in use";
            case WSAEADDRNOTAVAIL:
                return "Cannot assign requested address";
            case WSAENETDOWN:
                return "Network is down";
            case WSAENETUNREACH:
                return "Network is unreachable";
            case WSAENETRESET:
                return "Network dropped connection on reset";
            case WSAECONNABORTED:
                return "Software caused connection abort";
            case WSAECONNRESET:
                return "Connection reset by peer";
            case WSAENOBUFS:
                return "No buffer space available";
            case WSAEISCONN:
                return "Socket is already connected";
            case WSAENOTCONN:
                return "Socket is not connected";
            case WSAESHUTDOWN:
                return "Cannot send after socket shutdown";
            case WSAETOOMANYREFS:
                return "Too many references";
            case WSAETIMEDOUT:
                return "Connection timed out";
            case WSAECONNREFUSED:
                return "Connection refused";
            case WSAELOOP:
                return "Cannot translate name";
            case WSAENAMETOOLONG:
                return "Name too long";
            case WSAEHOSTDOWN:
                return "Host is down";
            case WSAEHOSTUNREACH:
                return "No route to host";
            case WSAENOTEMPTY:
                return "Directory not empty";
            case WSAEPROCLIM:
                return "Too many processes";
            case WSAEUSERS:
                return "User quota exceeded";
            case WSAEDQUOT:
                return "Disk quota exceeded";
            case WSAESTALE:
                return "Stale file handle reference";
            case WSAEREMOTE:
                return "Item is remote";
            case WSASYSNOTREADY:
                return "Network subsystem is unavailable";
            case WSAVERNOTSUPPORTED:
                return "Winsock.dll version out of range";
            case WSANOTINITIALISED:
                return "Successful WSAStartup not yet performed";
            case WSAEDISCON:
                return "Graceful shutdown in progress";
            case WSAENOMORE:
                return "No more results";
            case WSAECANCELLED:
                return "Call has been canceled";
            case WSAEINVALIDPROCTABLE:
                return "Procedure call table is invalid";
            case WSAEINVALIDPROVIDER:
                return "Service provider is invalid";
            case WSAEPROVIDERFAILEDINIT:
                return "Service provider failed to initialize";
            case WSASYSCALLFAILURE:
                return "System call failure";
            case WSASERVICE_NOT_FOUND:
                return "Service not found";
            case WSATYPE_NOT_FOUND:
                return "Class type not found";
            case WSA_E_NO_MORE:
                return "No more results";
            case WSA_E_CANCELLED:
                return "Call was canceled";
            case WSAEREFUSED:
                return "Database query was refused";
            case WSAHOST_NOT_FOUND:
                return "Host not found";
            case WSATRY_AGAIN:
                return "Nonauthoritative host not found";
            case WSANO_RECOVERY:
                return "This is a nonrecoverable error";
            case WSANO_DATA:
                return "Valid name, no data record of requested type";
            case WSA_QOS_RECEIVERS:
                return "QoS receivers";
            case WSA_QOS_SENDERS:
                return "QoS senders";
            case WSA_QOS_NO_SENDERS:
                return "No QoS senders";
            case WSA_QOS_NO_RECEIVERS:
                return "QoS no receivers";
            case WSA_QOS_REQUEST_CONFIRMED:
                return "QoS request confirmed";
            case WSA_QOS_ADMISSION_FAILURE:
                return "QoS admission error";
            case WSA_QOS_POLICY_FAILURE:
                return "QoS policy failure";
            case WSA_QOS_BAD_STYLE:
                return "QoS bad style";
            case WSA_QOS_BAD_OBJECT:
                return "QoS bad object";
            case WSA_QOS_TRAFFIC_CTRL_ERROR:
                return "QoS traffic control error";
            case WSA_QOS_GENERIC_ERROR:
                return "QoS generic error";
            case WSA_QOS_ESERVICETYPE:
                return "QoS service type error";
            case WSA_QOS_EFLOWSPEC:
                return "QoS flowspec error";
            case WSA_QOS_EPROVSPECBUF:
                return "Invalid QoS provider buffer";
            case WSA_QOS_EFILTERSTYLE:
                return "Invalid QoS filter style";
            case WSA_QOS_EFILTERTYPE:
                return "Invalid QoS filter type";
            case WSA_QOS_EFILTERCOUNT:
                return "Incorrect QoS filter count";
            case WSA_QOS_EOBJLENGTH:
                return "Invalid QoS object length";
            case WSA_QOS_EFLOWCOUNT:
                return "Incorrect QoS flow count";
            case WSA_QOS_EUNKOWNPSOBJ:
                return "Unrecognized QoS object";
            case WSA_QOS_EPOLICYOBJ:
                return "Invalid QoS policy object";
            case WSA_QOS_EFLOWDESC:
                return "Invalid QoS flow descriptor";
            case WSA_QOS_EPSFLOWSPEC:
                return "Invalid QoS provider-specific flowspec";
            case WSA_QOS_EPSFILTERSPEC:
                return "Invalid QoS provider-specific filterspec";
            case WSA_QOS_ESDMODEOBJ:
                return "Invalid QoS shape discard mode object";
            case WSA_QOS_ESHAPERATEOBJ:
                return "Invalid QoS shaping rate object";
            case WSA_QOS_RESERVED_PETYPE:
                return "Reserved policy QoS element type";
            default:
                return "No error";
           }
        );
        _____UNIX_CODE____(
            return strerror(errno)
        );
    }

    int32_t open_tcp(inet_family family) {
        int sock = socket(family, SOCK_STREAM, 0);

        if (sock == -1)
            throw_except<socket_exception>("Socket open error: %s", __sock_error_str__);
        
        return sock;
    }

    void close(int32_t sock) {
        _____WIN_CODE____(
            if (closesocket(sock) != 0)
                throw_except<socket_exception>("Socket close error: %s", __sock_error_str__);
        );
        _____UNIX_CODE____(
            ::close(sock);
        );
    }

    void bind(int32_t sock, const inet_address& address, int32_t port) {
        if (address.get_family() == inet_family::IPV4) {
            sockaddr_in addr;
            std::memset(&addr, 0, sizeof(addr));
            addr.sin_family = address.get_family();
            addr.sin_port   = htons(port);
            addr.sin_addr   = *address.get_IPv4();
            if (::bind(sock, (sockaddr*) &addr, sizeof(addr)) != 0)
                throw_except<bind_exception>("Socket bind error: %s", __sock_error_str__);
        } 
        
        else if (address.get_family() == inet_family::IPV6){
            sockaddr_in6 addr;
            std::memset(&addr, 0, sizeof(addr));
            addr.sin6_family = address.get_family();
            addr.sin6_port   = htons(port);
            addr.sin6_addr   = *address.get_IPv6();
            if (::bind(sock, (sockaddr*) &addr, sizeof(addr)) != 0)
                throw_except<bind_exception>("Socket bind error: %s", __sock_error_str__);
        } else {
            throw_except<illegal_argument_exception>("Bind: Wrong address family");
        }
    }

    void backlog(int32_t sock, int32_t maxq) {
        if (::listen(sock, maxq) != 0)
            throw_except<socket_exception>("Socket backlog error: %s", __sock_error_str__);
    }

    int32_t accept(int32_t serv_sock, socket_address* client_addr) {
        sockaddr_storage addr_storage;
        socklen_t size = sizeof(addr_storage);
        
        int32_t client_socket = ::accept(serv_sock, reinterpret_cast<sockaddr*>(&addr_storage), &size);
        
        _____WIN_CODE____(
            if (client_socket == ___null_socket___) {
                if (WSAGetLastError() == WSAEWOULDBLOCK)
                    return -1;
                else
                    throw_except<socket_exception>("Socket accept error: %s", __sock_error_str__);
            }
        );
    
        _____UNIX_CODE____(
            if (client_socket == ___null_socket___) {
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                    return -1;
                else
                    throw_except<socket_exception>("Socket accept error: %s", __sock_error_str__);
            }
        );

        {
            if (size == sizeof(sockaddr_in)) {
                sockaddr_in* a = reinterpret_cast<sockaddr_in*>(&addr_storage);
                *client_addr = socket_address(inet_address(a->sin_addr), ntohs(a->sin_port));
            } 
            
            else if (size == sizeof(sockaddr_in6)) {
                sockaddr_in6* a = reinterpret_cast<sockaddr_in6*>(&addr_storage);
                *client_addr = socket_address(inet_address(a->sin6_addr), ntohs(a->sin6_port));
            }

            else
                throw_except<socket_exception>("Unsupported address family type");
        }

        return client_socket;
    }
    
    void set_blocking(int32_t sock, bool block_mode) {
        _____WIN_CODE____(
            u_long mode = block_mode ? 0 : 1;
            if (ioctlsocket(sock, FIONBIO, &mode) != 0)
                throw_except<socket_exception>("Socket set_blocking error: %s", __sock_error_str__);
        );
        _____UNIX_CODE____(
            int flags = fcntl(sock, F_GETFL, 0);
            if (flags == -1) 
                throw_except<socket_exception>("Socket set_blocking: (F_GETFL) error: %s", __sock_error_str__);
            
            flags = block_mode ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
            
            if(fcntl(sock, F_SETFL, flags) != 0)
                throw_except<socket_exception>("Socket set_blocking: (F_SETFL) error: %s", __sock_error_str__);
        );
    }

    void connect(int32_t client_sock, const inet_address& address, int32_t port) {
        if (address.get_family() == inet_family::IPV4) {
            sockaddr_in addr;
            std::memset(&addr, 0, sizeof(addr));
            addr.sin_family = address.get_family();
            addr.sin_addr   = *address.get_IPv4();
            addr.sin_port   = htons(port);
            if (::connect(client_sock, (sockaddr*) &addr, sizeof(addr)) != 0)
                throw_except<connect_exception>("Connect fail: %s", __sock_error_str__);
        } 
        
        else if (address.get_family() == inet_family::IPV6) {
            sockaddr_in6 addr;
            std::memset(&addr, 0, sizeof(addr));
            addr.sin6_family    = address.get_family();
            addr.sin6_addr      = *address.get_IPv6();
            addr.sin6_port      = htons(port);
            if (::connect(client_sock, (sockaddr*)&addr, sizeof(addr)) != 0)
                throw_except<connect_exception>("Connect fail: %s", __sock_error_str__);
        } 
        
        else {
            throw_except<illegal_argument_exception>("Connect: Wrong address family");
        }
    }
 
    int64_t send(int32_t sock, const char* data, int64_t length, bool is_blocking) {
        int64_t sended = 0;
        _____WIN_CODE____(
            SetLastError(ERROR_SUCCESS);
            sended = ::send(sock, data, length, 0);
            if (sended == SOCKET_ERROR) {
                int err = GetLastError();
                if (err == WSAEWOULDBLOCK)
                    return 0;
                else if (err == WSAETIMEDOUT && is_blocking)
                    throw_except<socket_timeout_exception>(__sock_error_str__);
                else if (err == WSAECONNRESET)
                    throw_except<connect_exception>("Connection is closed by remote side");
                throw_except<io_exception>(__sock_error_str__);
            } 
        );
        
        _____UNIX_CODE____(
            errno = 0;
            sended = ::send(sock, data, length, MSG_NOSIGNAL);
            if (sended == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    if (is_blocking) 
                        throw_except<socket_timeout_exception>(__sock_error_str__);
                    return 0;
                } 
                
                else if (errno == EPIPE) 
                    throw_except<connect_exception>("Connection is closed by remote side");

                throw_except<io_exception>(__sock_error_str__);
            } 
        );
        
        return sended;
    }

    int64_t recv(int32_t sock, char* data, int64_t length, bool is_blocking) {
        int64_t received = 0;
        _____WIN_CODE____(
            SetLastError(ERROR_SUCCESS);
            received = ::recv(sock, data, length, 0);
            if (received == SOCKET_ERROR) {
                int err = GetLastError();
                if (err == WSAEWOULDBLOCK)
                    return 0;
                else if (err == WSAETIMEDOUT && is_blocking)
                    throw_except<socket_timeout_exception>(__sock_error_str__);
                else if (err == WSAECONNRESET)
                    throw_except<connect_exception>("Connection is closed by remote side");
            }

            else if (received == 0)
                throw_except<connect_exception>("Connection is closed by remote side");
        );

        _____UNIX_CODE____(
            errno = 0;
            received = ::recv(sock, data, length, 0);
            if (received == -1) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    if (is_blocking)
                        throw_except<socket_timeout_exception>(__sock_error_str__);    
                    return 0;
                }
                throw_except<io_exception>(__sock_error_str__);
            } 
            
            else if (received == 0) {
                throw_except<connect_exception>("Connection is closed by remote side");
            }
        );
        return received;
    }

    void get_sock_name(int32_t sock, socket_address& address) {
        sockaddr_storage addr_storage;
        socklen_t len = sizeof(addr_storage);
        std::memset(&addr_storage, 0, len);
        if (getsockname(sock, reinterpret_cast<sockaddr*>(&addr_storage), &len) != 0)
            throw_except<socket_exception>(__sock_error_str__);

        if (len == sizeof(sockaddr_in)) {
            sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(&addr_storage);
            address = socket_address(inet_address(addr->sin_addr), ntohs(addr->sin_port));
        } 
        
        else if (len == sizeof(sockaddr_in6)) {
            sockaddr_in6* addr = reinterpret_cast<sockaddr_in6*>(&addr_storage);
            address = socket_address(inet_address(addr->sin6_addr), ntohs(addr->sin6_port));
        } 
        
        else {
            throw_except<unsupported_operation_exception>("Unsupported address family type");
        }
    }

    void shutdown_in(int32_t sock) {
        if (::shutdown(sock, SHUT_RD) != 0)
            throw_except<socket_exception>("Shutdown read: %s", __sock_error_str__);
    }
    
    void shutdown_out(int32_t sock) {
        if (::shutdown(sock, SHUT_WR) != 0)
            throw_except<socket_exception>("Shutdown write: %s", __sock_error_str__);
    }

#define ___to_string___(str) #str

/**
 * ============================================================= S E T ===========================================================================
 */
#define ___make_set_socket_func_int__(__func_name__, level, optID)                                                      \
                void __func_name__(int32_t sock, const socket_option& value) {                                          \
                    int val = value.int_value;                                                                          \
                    if (setsockopt(sock, level, optID, (const char*) &val, sizeof(val)) != 0)                           \
                        throw_except<socket_exception>(___to_string___(__func_name__ fail: %s), __sock_error_str__);    \
                }       

#define ___make_set_socket_func_timeval__(__func_name__, level, optID)                                                                  \
                void __func_name__(int32_t sock, const socket_option& value) {                                                          \
                    _____WIN_CODE____(                                                                                                  \
                        DWORD ms = value.timeout.millis;                                                                                \
                        if (setsockopt(sock, level, optID, (const char*) &ms, sizeof(ms)) != 0)                                         \
                            throw_except<socket_exception>(___to_string___(__func_name__ fail: %s), __sock_error_str__);                \
                    );                                                                                                                  \
                                                                                                                                        \
                    _____UNIX_CODE____(                                                                                                 \
                        struct timeval tv;                                                                                              \
                        ms_to_timeval(&tv, value.timeout.millis);                                                                       \
                        if (setsockopt(sock, level, optID, (const char*) &tv, sizeof(tv)) != 0)                                         \
                            throw_except<socket_exception>(___to_string___(__func_name__ fail: %s), __sock_error_str__);                \
                    );                                                                                                                  \
                }                                                                                                                       \

/**
 * set bool options
 */
___make_set_socket_func_int__(set_tcp_no_delay,         IPPROTO_TCP,    TCP_NODELAY)
___make_set_socket_func_int__(set_reuse_addr,           SOL_SOCKET,     SO_REUSEADDR)
___make_set_socket_func_int__(set_broadcast,            SOL_SOCKET,     SO_BROADCAST)
___make_set_socket_func_int__(set_oobinline,            SOL_SOCKET,     SO_OOBINLINE)
___make_set_socket_func_int__(set_receive_buffer_size,  SOL_SOCKET,     SO_RCVBUF);
___make_set_socket_func_int__(set_send_buffer_size,     SOL_SOCKET,     SO_RCVBUF);
___make_set_socket_func_int__(set_type_of_service,      IPPROTO_TCP,    IP_TOS);
___make_set_socket_func_int__(set_keep_alive,           SOL_SOCKET,     SO_KEEPALIVE);

/**
 * set timeval options
 */
 ___make_set_socket_func_timeval__(set_send_timeout,    SOL_SOCKET, SO_SNDTIMEO)
 ___make_set_socket_func_timeval__(set_receive_timeout, SOL_SOCKET, SO_RCVTIMEO)

/**
 * ============================================================= G E T ===========================================================================
 */
#define ___make_get_socket_func_int__(__func_name__, level, optID)                                                                  \
                            void __func_name__(int32_t sock, socket_option& value) {                                                \
                                int val         = 0;                                                                                \
                                socklen_t len   = sizeof(val);                                                                      \
                                if (getsockopt(sock, level, optID, (char*) &val, &len) != 0)                                        \
                                    throw_except<socket_exception>(___to_string___(__func_name__ fail: %s), __sock_error_str__);    \
                                value.int_value = val;                                                                              \
                            }     

#define ___make_get_socket_func_timeval__(__func_name__, level, optID)                                                                  \
                            void __func_name__(int32_t sock, socket_option& value) {                                                    \
                                _____WIN_CODE____(                                                                                      \
                                    DWORD ms = 0;                                                                                       \
                                    socklen_t len = sizeof(ms);                                                                         \
                                    if (getsockopt(sock, level, optID, (char*) &ms, &len) != 0)                                         \
                                        throw_except<socket_exception>(___to_string___(__func_name__ fail: %s), __sock_error_str__);    \
                                    value.timeout.millis = ms;                                                                          \
                                );                                                                                                      \
                                                                                                                                        \
                                _____UNIX_CODE____(                                                                                     \
                                    struct timeval tv;                                                                                  \
                                    std::memset(&tv, 0, sizeof(tv));                                                                    \
                                    socklen_t len = sizeof(tv);                                                                         \
                                    if (getsockopt(sock, level, optID, (char*) &tv, &len) != 0)                                         \
                                        throw_except<socket_exception>(___to_string___(__func_name__ fail: %s), __sock_error_str__);    \
                                    value.timeout.millis = timeval_to_ms(&tv);                                                          \
                                );                                                                                                      \
                                                                                                                                        \
                            }                                                                                                           \

/**
 * get bool options
 */
___make_get_socket_func_int__(get_tcp_no_delay,        IPPROTO_TCP,    TCP_NODELAY)
___make_get_socket_func_int__(get_reuse_addr,          SOL_SOCKET,     SO_REUSEADDR)
___make_get_socket_func_int__(get_broadcast,           SOL_SOCKET,     SO_BROADCAST)
___make_get_socket_func_int__(get_oobinline,           SOL_SOCKET,     SO_OOBINLINE)
___make_get_socket_func_int__(get_receive_buffer_size,  SOL_SOCKET,     SO_RCVBUF);
___make_get_socket_func_int__(get_send_buffer_size,     SOL_SOCKET,     SO_RCVBUF);
___make_get_socket_func_int__(get_type_of_service,      IPPROTO_TCP,    IP_TOS);
___make_get_socket_func_int__(get_keep_alive,           SOL_SOCKET,     SO_KEEPALIVE);

___make_get_socket_func_timeval__(get_send_timeout,    SOL_SOCKET, SO_SNDTIMEO)
___make_get_socket_func_timeval__(get_receive_timeout, SOL_SOCKET, SO_RCVTIMEO)


 

    void set_linger(int32_t sock, const socket_option& value) {
        struct linger l;
        l.l_linger  = value.linger.sec_time;
        l.l_onoff   = value.linger.on_off;
        if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*) &l, sizeof(l)) != 0)
            throw_except<socket_exception>("set_linger fail: %s", __sock_error_str__);
    }

    void get_linger(int32_t sock, socket_option& value) {
        struct linger l;
        socklen_t len = sizeof(l);
        std::memset(&l, 0, len);
        if (getsockopt(sock, SOL_SOCKET, SO_LINGER, (char*) &l, &len) != 0)
            throw_except<socket_exception>("set_linger fail: %s", __sock_error_str__);
        value.linger.on_off     = l.l_onoff;
        value.linger.sec_time   = l.l_linger;
    }

    void set_keep_alive_ex(int32_t sock, const socket_option& value) {
        
        if (!value.keepalive.on_off) {
            set_keep_alive(sock, {.int_value = false});
            return;
        } else {
            set_keep_alive(sock, {.int_value = true});
        }

        _____UNIX_CODE____(
            int idle        = value.keepalive.time_to_first;
            int interval    = value.keepalive.time_interval;
            int count       = value.keepalive.try_count;

            if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle)) != 0)
                throw_except<socket_exception>("set_keep_alive_ex fail: %s", __sock_error_str__);
            
            if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) != 0) 
                throw_except<socket_exception>("set_keep_alive_ex fail: %s", __sock_error_str__);
            
            if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count)) != 0) 
                throw_except<socket_exception>("set_keep_alive_ex fail: %s", __sock_error_str__);
        );

        _____WIN_CODE____(
            struct tcp_keepalive ka;
            ka.onoff                = 1; 
            ka.keepalivetime        = value.keepalive.time_to_first;
            ka.keepaliveinterval    = value.keepalive.time_interval;
        
            DWORD bytesReturned;
            if (WSAIoctl(sock, SIO_KEEPALIVE_VALS, &ka, sizeof(ka), NULL, 0, &bytesReturned, NULL, NULL) != 0)
                throw_except<socket_exception>("set_keep_alive_ex fail: %s", __sock_error_str__);
        );
    }

    void set_sock_opt(int32_t sock, int32_t opt, const socket_option& value) {
        switch(opt) {
            case socket_option::bsd::TCP_NODELAY_:
                set_tcp_no_delay(sock, value);
                return;
            case socket_option::bsd::SO_REUSEADDR_:
                set_reuse_addr(sock, value);
                return;
            case socket_option::bsd::SO_BROADCAST_:
                set_broadcast(sock, value);
                return;
            case socket_option::bsd::IP_TOS_:
                set_type_of_service(sock, value);
                return;
            case socket_option::bsd::SO_LINGER_:
                set_linger(sock, value);
                return;
            case socket_option::bsd::SO_RCVTIMEO_:
                set_receive_timeout(sock, value);
                return;
            case socket_option::bsd::SO_SNDTIMEO_:
                set_send_timeout(sock, value);
                return;
            case socket_option::bsd::SO_RCVBUF_:
                set_receive_buffer_size(sock, value);
                return;
            case socket_option::bsd::SO_SNDBUF_:
                set_send_buffer_size(sock, value);
                return;
            case socket_option::bsd::SO_KEEPALIVE_:
                set_keep_alive_ex(sock, value);
                return;
            case socket_option::bsd::SO_OOBINLINE_:
                set_oobinline(sock, value);
                return;
            default:
                throw_except<illegal_argument_exception>("Option %i is invalid", (int) opt);
        }
    }



    void get_sock_opt(int32_t sock, int32_t opt, socket_option& value) {
        switch(opt) {
            case socket_option::bsd::TCP_NODELAY_:
                get_tcp_no_delay(sock, value);
                return;
            case socket_option::bsd::SO_REUSEADDR_:
                get_reuse_addr(sock, value);
                return;
            case socket_option::bsd::SO_BROADCAST_:
                get_broadcast(sock, value);
                return;
            case socket_option::bsd::IP_TOS_:
                get_type_of_service(sock, value);
                return;
            case socket_option::bsd::SO_LINGER_:
                get_linger(sock, value);
                return;
            case socket_option::bsd::SO_RCVTIMEO_:
                get_receive_timeout(sock, value);
                return;
            case socket_option::bsd::SO_SNDTIMEO_:
                get_send_timeout(sock, value);
                return;
            case socket_option::bsd::SO_RCVBUF_:
                get_receive_buffer_size(sock, value);
                return;
            case socket_option::bsd::SO_SNDBUF_:
                get_send_buffer_size(sock, value);
                return;
            case socket_option::bsd::SO_OOBINLINE_:
                get_oobinline(sock, value);
                return;
            case socket_option::bsd::SO_KEEPALIVE_:
                get_keep_alive(sock, value);
                return;
            default:
                throw_except<illegal_argument_exception>("Option %i is invalid!", (int) opt);
        }
    }
}//bsd_socket
}//jstd