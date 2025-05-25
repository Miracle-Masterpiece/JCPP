#ifndef _JSTD_CPP_LANG_NET_INET_ADDRESS_H_
#define _JSTD_CPP_LANG_NET_INET_ADDRESS_H_

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

#include <cstdint>
#include <cpp/lang/utils/optional.hpp>

namespace jstd {

enum inet_family {
    NONE = 0, IPV4 = AF_INET, IPV6 = AF_INET6
};

class inet_address {
    struct jsl_in6_addr{
        union{
            uint8_t	    _byte[sizeof(in6_addr)];
            uint16_t    _short[8];
            uint32_t    _int[4];
        };
    };
protected:
    static const int64_t HOST_BUFFER_SIZE     = 64;
    union {
        union{
            in_addr IPv4;
            in6_addr IPv6;
        };
        char _addrBuf[(sizeof(IPv4) < sizeof(IPv6)) ? sizeof(IPv6) : sizeof(IPv4)];
    };
    char hostName[HOST_BUFFER_SIZE];
    inet_family family;
    static const char* getStringError();
    static const char* getGaiStringError(int errorCode);
public:
    inet_address();
    inet_address(inet_family family);
    inet_address(const char* ip);
    inet_address(const in6_addr& IPv6, const char* hostname = nullptr);
    inet_address(const in_addr& IPv4, const char* hostname = nullptr);
    inet_address(const inet_address& addr);
    inet_address(inet_address&& addr);
    inet_address& operator= (const inet_address& addr);
    inet_address& operator= (inet_address&& addr);
    
    virtual ~inet_address();
    
    inet_family get_family() const;
    const in6_addr* get_IPv6() const;
    const in_addr* get_IPv4() const;
    int to_string(char buf[], int bufsize) const;
	const char* get_host_name() const;

    int64_t hashcode() const;
    bool equals(const inet_address& addr) const;

    static optional<inet_address> get_by_name(const char* domain);
    
    /**
     * @throws illegal_state_exception
     *      Если произошла ошибка при получении адресов.
     */
    static int get_all_by_name(inet_address buf[], int bufsize, const char* domain);

    static inet_address as_IPv4(const uint8_t buf[], int bufsize);
    static inet_address as_IPv6(const uint16_t buf[], int bufsize);
	
	static inet_address parse_IPv4(const char* ip);
    static inet_address parse_IPv6(const char* ip);
    
    static inet_address localhost(inet_family family);

    static int IPv4ToString(char buf[], int bufsize, const inet_address& address);
    static int IPv6ToString(char buf[], int bufsize, const inet_address& address);
};

class socket_address {
    inet_address _address;
    int32_t      _port;
public:
    socket_address();
    socket_address(const inet_address& address, int32_t port);  
    socket_address(const socket_address&);
    socket_address(socket_address&&);
    socket_address& operator= (const socket_address&);
    socket_address& operator= (socket_address&&);

    int32_t get_port() const;
    const inet_address& get_address() const;
    
    int64_t hashcode() const;
    bool equals(const socket_address& sock_addr) const;
    int to_string(char buf[], int bufsize);

};


template<typename T>
struct hash_for;

template<typename T>
struct equal_to;

template<>
struct equal_to<inet_address> {
    bool operator() (const inet_address& i1, const inet_address& i2) {
        return i1.equals(i2);
    }
};

template<>
struct hash_for<inet_address> {
    uint64_t operator() (const inet_address& i) {
        return i.hashcode();
    }
};

template<>
struct equal_to<socket_address> {
    bool operator() (const socket_address& i1, const socket_address& i2) {
        return i1.equals(i2);
    }
};

template<>
struct hash_for<socket_address> {
    uint64_t operator() (const socket_address& i) {
        return i.hashcode();
    }
};

}

#endif//_JSTD_CPP_LANG_NET_INET_ADDRESS_H_