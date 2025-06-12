#include <cpp/lang/net/inetaddr.hpp>
#define __PUSH_STACK__

#if defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC)
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cerrno>
#elif defined(JSTD_OS_WINDOWS)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else 
#error Unsupported platform!
#endif

#include <cstring>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/string.hpp>
#include <allocators/inline_linear_allocator.hpp>
#include <allocators/ArrayList.h>
#include <cpp/lang/array.hpp>

#if defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC)
#define UNIX_CODE(code) code
#define WIN_CODE(code)
#elif defined(JSTD_OS_WINDOWS)
#define UNIX_CODE(code)
#define WIN_CODE(code) code
#endif

namespace jstd {

enum ip_token {
    L_NUMBER, D_NUMBER, IDENTIFICATOR, COLON, COLON2, DOT
};

struct ip_token_storage {
    ip_token m_ip_token;
    union{
        long    long_num;
        double  double_num;
    };
    ip_token_storage() {}
    ip_token_storage(ip_token t) : m_ip_token(t) {
        
    }
};

class ip_parser {
    tca::base_allocator*            _allocator;
    tca::array_list<ip_token_storage>  _ip_tokens;
    const char* _data;
    int         _offset;
    int         _length;
public:
    ip_parser(tca::base_allocator* _allocator, const char* ip_string, int str_len = -1);
    char next();
    bool eof() const;
    const tca::array_list<ip_token_storage>& ip_token_list() const;
    void tokinize(inet_family family);
    inet_address make_v4() const;
    inet_address make_v6() const;
};
    
    ip_parser::ip_parser(tca::base_allocator* allocator, const char* ip_string, int str_len) : 
    _allocator(allocator), 
    _ip_tokens(allocator, 32),
    _data(ip_string), 
    _offset(0), 
    _length(str_len < 0 ? std::strlen(ip_string) : str_len) {
        
    }

    char ip_parser::next() {
        if (eof())
            return 0;
        return _data[_offset++];
    }

    bool ip_parser::eof() const {
        return _offset > _length;
    }
    
    const tca::array_list<ip_token_storage>& ip_parser::ip_token_list() const {
        return _ip_tokens;
    }

    void ip_parser::tokinize(inet_family family) {
        char c = next();
        tca::inline_linear_allocator<64> allocator;
        jstd::array<char> numbers(64, &allocator);
        while (!eof()) {
            if (c == ':') {
                if (family == inet_family::IPV4)
                    throw_except<illegal_argument_exception>("IPv4 [ %s ] is not valid!", _data);
                char nxt = next();
                if (nxt != ':') {
                    c = nxt;
                    _ip_tokens.add(ip_token_storage(ip_token::COLON));
                } else {
                    c = next();
                    _ip_tokens.add(ip_token_storage(ip_token::COLON2));
                }
                continue;
            } 
            
            else if (c == '.') {
                _ip_tokens.add(ip_token_storage(ip_token::DOT));
                c = next();
                continue;
            }
            
            else if (std::isxdigit(c)) {
                int off = 0;
                do {
                    numbers[off++] = c;
                    c = next();
                } while (std::isxdigit(c));
                numbers[off] = '\0';
                ip_token_storage tok;
                tok.m_ip_token     = ip_token::L_NUMBER;
                tok.long_num    = family == inet_family::IPV6 ? strtol(numbers.data(), nullptr, 16) : atol(numbers.data());
                _ip_tokens.add(tok);
                continue;
            }
            throw_except<illegal_argument_exception>("IPv6: %s is not valid!", _data);
        }   
    }
    
    inet_address ip_parser::make_v4() const {
        uint8_t buf[4];
        jstd::array<uint8_t> ip(buf, sizeof(buf) / sizeof(uint8_t));
        int offset = 0;
        for (int i = 0, len = _ip_tokens.size(); i < len; ++i) {
            if (_ip_tokens.at(i).m_ip_token == ip_token::L_NUMBER)
                ip[offset++] = (_ip_tokens.at(i).long_num & 0xFF);
        }
        return inet_address::as_IPv4(ip.data(), ip.length);
    }

#define ____EXPR_LEAVE____(what_increment)                                          \
                            if (_ip_tokens.at(i).m_ip_token == ip_token::COLON2)             \
                                break;                                              \
                            else if (_ip_tokens.at(i).m_ip_token == ip_token::L_NUMBER)      \
                                ++what_increment                                    \

    inet_address ip_parser::make_v6() const {
        
        int begin_sections  = 0;
        int end_sections    = 0;

        uint16_t buf[8] = {0};
        array<uint16_t> ip(buf, sizeof(buf) / sizeof(uint16_t));
        
        for (int i = 0, len = _ip_tokens.size(); i < len; ++i) {
            ____EXPR_LEAVE____(begin_sections);
            if (i + 1 >= len)
                if (begin_sections < 8)
                    throw_except<illegal_argument_exception>("IPv6: %s is not valid!", _data);
        }

        if (begin_sections < 8) {
            for (int i = _ip_tokens.size() - 1; i >= 0; --i) {
                ____EXPR_LEAVE____(end_sections);
            }
        }

        int offset = 0;
        for (int i = 0, len = _ip_tokens.size(); i < len; ++i) {
            if (_ip_tokens.at(i).m_ip_token == ip_token::COLON) { 
                continue;
            } 
            
            else if (_ip_tokens.at(i).m_ip_token == ip_token::COLON2) {
                offset += (8 - (begin_sections + end_sections));
            }
            
            else if (_ip_tokens.at(i).m_ip_token == ip_token::L_NUMBER) {
                ip[offset++] = _ip_tokens.at(i).long_num;
            }        
        }

        return inet_address::as_IPv6(ip.data(), ip.length);
    }
    

















  /*static */ const char* getStringError() {
    __PUSH_STACK__
    #if defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC) || defined(JSTD_OS_UNIX)
            return strerror(errno);
    #elif _WIN32
            int error = WSAGetLastError();
            switch(error){
                case 0:
                    return "No error!";
                case WSAEAFNOSUPPORT: 
                    return "Inet family type is not suppoted!";
                case ERROR_INVALID_PARAMETER : 
                    return "Invalid parameter! Buffer overflow or buffer length equal zero!";
            }
            return "InetAddress unknow error!";
    #endif
    }
    
    //У меня нет ни малейшего понятия, что значит Gai в названии функции.
    //Two tausen years later: Gai = это GetAddrInfo
    /*static*/ const char* getGaiStringError(int errorCode) {
#if defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC) || defined(JSTD_OS_UNIX)
        return gai_strerror(errorCode);
#elif defined(JSTD_OS_WINDOWS)
        switch (errorCode) {
            case 0: 
                return "No error!";
            case WSATRY_AGAIN:  // EAI_AGAIN
                return "A temporary failure in name resolution occurred.";
            case WSAEINVAL:  // EAI_BADFLAGS
                return "An invalid value was provided for the ai_flags member of the pHints parameter.";
            case WSANO_RECOVERY:  // EAI_FAIL
                return "A nonrecoverable failure in name resolution occurred.";
            case WSAEAFNOSUPPORT:  // EAI_FAMILY
                return "The ai_family member of the pHints parameter is not supported.";
            case WSA_NOT_ENOUGH_MEMORY:  // EAI_MEMORY
                return "A memory allocation failure occurred.";
            case WSAHOST_NOT_FOUND:  // EAI_NONAME
                return "The name does not resolve for the supplied parameters or the pNodeName and pServiceName parameters were not provided.";
            case WSATYPE_NOT_FOUND:  // EAI_SERVICE
                return "The pServiceName parameter is not supported for the specified ai_socktype member of the pHints parameter.";
            case WSAESOCKTNOSUPPORT:  // EAI_SOCKTYPE
                return "The ai_socktype member of the pHints parameter is not supported.";
            case WSANOTINITIALISED :
                return "Windows sockets is not initialized.";
        }
        return "InetAddress unknow error!";
#endif
    }


    /*static*/ const inet_family inet_family::NONE = {0};
    /*static*/ const inet_family inet_family::IPV4(AF_INET);
    /*static*/ const inet_family inet_family::IPV6(AF_INET6);

    inet_address::inet_address() : IPv4(), family(inet_family::IPV4) {
        __PUSH_STACK__
        std::memset(m_data_buffer, 0, sizeof(m_data_buffer));
        m_host_name[0] = '\0';
    }

    inet_address::inet_address(inet_family family) : inet_address() {
        __PUSH_STACK__
        this->family = family;
    }

    inet_address::inet_address(const inet_address& addr) : IPv4(), family(addr.family) {
        __PUSH_STACK__
        std::memcpy(m_host_name, addr.m_host_name, sizeof(addr.m_host_name));
        std::memcpy(m_data_buffer, addr.m_data_buffer, sizeof(addr.m_data_buffer));
    }
    
    inet_address::inet_address(inet_address&& addr) : IPv4(), family(addr.family) {
        __PUSH_STACK__
        std::memcpy(m_host_name, addr.m_host_name, sizeof(addr.m_host_name));
        std::memcpy(m_data_buffer, addr.m_data_buffer, sizeof(addr.m_data_buffer));
    }

    inet_address& inet_address::operator= (const inet_address& addr) {
        __PUSH_STACK__
        if (&addr != this){
            std::memcpy(m_host_name, addr.m_host_name, sizeof(addr.m_host_name));
            std::memcpy(m_data_buffer, addr.m_data_buffer, sizeof(addr.m_data_buffer));
            family = addr.family;
        }
        return *this;
    }
    
    inet_address& inet_address::operator= (inet_address&& addr) {
        __PUSH_STACK__
        if (&addr != this){
            std::memcpy(m_host_name, addr.m_host_name, sizeof(addr.m_host_name));
            std::memcpy(m_data_buffer, addr.m_data_buffer, sizeof(addr.m_data_buffer));
            family = addr.family;
        }
        return *this;
    }

    inet_address::~inet_address() {
        __PUSH_STACK__
    }

    //Эта чёртова функция не в классе строке, поскольку в данный момент мне всё-равно.
    bool contains(const char* str, char c) {
        __PUSH_STACK__
        for (int i = 0; ;++i) {
            if (str[i] == '\0')
                break;
            if (str[i] == c)
                return true;
        }
        return false;
    }

    inet_address::inet_address(const char* ip) {
        __PUSH_STACK__
        std::memset(m_host_name, 0, sizeof(m_host_name));
        if (contains(ip, '.')){
            family = inet_family::IPV4;
            (*this) = parse_IPv4(ip);
        }
        
        else if(contains(ip, ':')) {
            family = inet_family::IPV6;
            (*this) = parse_IPv6(ip);
        }
        
        else{
            throw_except<illegal_state_exception>("Illegal address: %s", ip);
        }
    }

    /*static*/ inet_address inet_address::as_IPv4(const uint8_t buf[], int bufsize) {
        __PUSH_STACK__
        if (bufsize < 4)
            throw_except<illegal_argument_exception>("bufsize must be >= 4");
        inet_address ip4;
        ip4.family = inet_family::IPV4;
        for (int i = 0; i < 4; ++i) {
            ip4.IPv4.m_byte_view[i] = buf[i];
        }
        return ip4;
    }

    /*static*/ inet_address inet_address::as_IPv6(const uint16_t buf[], int bufsize) {
        __PUSH_STACK__
        if (bufsize < 8)
            throw_except<illegal_argument_exception>("bufsize must be >= 8");
        inet_address ip6;
        ip6.family = inet_family::IPV6;
        for (int32_t i = 0; i < 8; ++i) {
            ip6.IPv6.m_short_view[i] = htons(buf[i]);
        }
        return ip6;
    }

    inet_address inet_address::parse_IPv4(const char* ip_string) {
        __PUSH_STACK__
        JSTD_DEBUG_CODE(
            if (ip_string == nullptr)
                throw_except<illegal_argument_exception>("ip_string == null");
        );
        tca::inline_linear_allocator<512> allocator;
        ip_parser parser(&allocator, ip_string);
        parser.tokinize(inet_family::IPV4);
        allocator.print();
        return parser.make_v4();
    }

    inet_address inet_address::parse_IPv6(const char* ip_string) {
        __PUSH_STACK__
        JSTD_DEBUG_CODE(
            if (ip_string == nullptr)
                throw_except<illegal_argument_exception>("ip_string == null");
        );
        tca::inline_linear_allocator<1024> allocator;
        ip_parser parser(&allocator, ip_string);
        parser.tokinize(inet_family::IPV6);
        allocator.print();
        return parser.make_v6();
    }

    int64_t inet_address::hashcode() const {
        
        if (family == inet_family::IPV4) {
            return objects::hashcode<char>(reinterpret_cast<const char*>(&IPv4), sizeof(IPv4));
        } 
        
        else if (family == inet_family::IPV6) {
            return objects::hashcode<char>(reinterpret_cast<const char*>(&IPv6), sizeof(IPv6));
        }

        return 0;
    }
    
    bool inet_address::equals(const inet_address& addr) const {
        if (&addr == this)
            return true;
        
        if (family == inet_family::IPV4 && addr.family == inet_family::IPV4) {
            return 
                    objects::equals<char>(reinterpret_cast<const char*>(&IPv4), reinterpret_cast<const char*>(&addr.IPv4), sizeof(IPv4)) 
                    &&
                    family == addr.family;
        } 
        
        else if (family == inet_family::IPV6 && addr.family == inet_family::IPV6) {
            return 
                    objects::equals<char>(reinterpret_cast<const char*>(&IPv6), reinterpret_cast<const char*>(&addr.IPv6), sizeof(IPv6))
                    &&
                    family == addr.family;
        }

        return false;
    }

    int inet_address::to_string(char buf[], int bufsize) const {
        __PUSH_STACK__
        if (family == inet_family::IPV4){
            return IPv4ToString(buf, bufsize, *this);
        }else{
            return IPv6ToString(buf, bufsize, *this);
        }
    }

    inet_family inet_address::get_family() const {
        return family;
    }

    const char* inet_address::get_host_name() const {
        return m_host_name;
    }

    /*static*/ int inet_address::IPv4ToString(char buf[], int bufsize, const inet_address& address){
        __PUSH_STACK__
        if (buf == nullptr)
            throw_except<null_pointer_exception>("buf is null!");
        const int BUF_SIZE = 16;
        char tmpBuf[BUF_SIZE];
        const uint32_t addr = address.IPv4.m_int_view;
        const uint8_t b1 =  addr          & 0xff;
        const uint8_t b2 = (addr >> 8)    & 0xff;
        const uint8_t b3 = (addr >> 16)   & 0xff;
        const uint8_t b4 = (addr >> 24)   & 0xff;
        const int32_t length = snprintf(tmpBuf, sizeof(tmpBuf), "%d.%d.%d.%d", b1, b2, b3, b4);
        if (bufsize < (length + 1))
            throw_except<overflow_exception>("bufsize < result length");
        std::memcpy(buf, tmpBuf, length);
        buf[length] = '\0';
        return length;
    }
        
    /*static*/ int inet_address::IPv6ToString(char buf[], int bufsize, const inet_address& address){
        __PUSH_STACK__
        const internal::net::ipv6_addr* addr = &address.IPv6;
        uint16_t values[8];
    
        for (int i = 0; i < 8; ++i)
            values[i] = ntohs(addr->m_short_view[i]);

        const int length = snprintf(buf, bufsize, "%x:%x:%x:%x:%x:%x:%x:%x", values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7]);
        if (bufsize < (length + 1))
            throw_except<overflow_exception>("bufsize < result length! [%i : %i]", bufsize, length + 1);

        return length;
    }

    /*static*/ optional<inet_address> inet_address::get_by_name(const char* domain) {
        __PUSH_STACK__
        inet_address addr;
        int count = get_all_by_name(&addr, 1, domain);
        return count > 0 ? addr : optional<inet_address>::null_opt();
    }

    inet_address inet_address::as_in_addr(const in_addr* addr_in) {
        JSTD_DEBUG_CODE(
            if (addr_in == nullptr)
                throw_except<illegal_argument_exception>("addr_in == null");
        );
        inet_address addr;
        addr.family             = inet_family::IPV4;
        addr.IPv4.m_int_view    = addr_in->s_addr;
        return addr;
    }
    
    inet_address inet_address::as_in6_addr(const in6_addr* addr_in) {
        JSTD_DEBUG_CODE(
            if (addr_in == nullptr)
                throw_except<illegal_argument_exception>("addr_in == null");
        );
        inet_address addr;
        addr.family = inet_family::IPV6;
        for (int i = 0; i < 16; ++i)
            addr.IPv6.m_byte_view[i] = addr_in->s6_addr[i];
        return addr;
    }

    void inet_address::get_in_addr(in_addr* addr_out) const {
        JSTD_DEBUG_CODE(
            if (addr_out == nullptr)
                throw_except<illegal_argument_exception>("addr_out == null");
            if (family != inet_family::IPV4)
                throw_except<illegal_state_exception>("family is not IPv4");
        );
        addr_out->s_addr = IPv4.m_int_view;
    }

    void inet_address::get_in6_addr(in6_addr* addr_out) const {
        JSTD_DEBUG_CODE(
            if (addr_out == nullptr)
                throw_except<illegal_argument_exception>("addr_out == null");
            if (family != inet_family::IPV6)
                throw_except<illegal_state_exception>("family is not IPv6");
        );
        for (int i = 0; i < 16; ++i) 
            addr_out->s6_addr[i] = IPv6.m_byte_view[i];
    }

    /*static*/ int inet_address::get_all_by_name(inet_address dst[], int bufsize, const char* domain) {
        __PUSH_STACK__
        addrinfo addrInf;
        memset(&addrInf, 0, sizeof(addrInf));
        addrInf.ai_protocol     = AF_UNSPEC;
        addrInf.ai_socktype     = SOCK_STREAM;
        addrInf.ai_flags       |= AI_CANONNAME;

        addrinfo* begin = nullptr;
        int errcode   = getaddrinfo(domain, null, &addrInf, &begin);
        
        if (errcode != 0)
            throw_except<illegal_state_exception>(getGaiStringError(errcode));    

        int off = 0;
        for (addrinfo* i = begin; i != nullptr && off < bufsize; i = i->ai_next) {

            inet_address addr;
            if (i->ai_family == inet_family::IPV4){
                const sockaddr_in* sock4  = (const sockaddr_in*) i->ai_addr;
                addr = inet_address::as_in_addr(&sock4->sin_addr);
            }
            
            else if(i->ai_family == inet_family::IPV6){
                const sockaddr_in6* sock6  = (const sockaddr_in6*) i->ai_addr;
                addr = inet_address::as_in6_addr(&sock6->sin6_addr);
            } 
            
            else {
                continue;
            }

            dst[off++] = addr;
        }
        
        freeaddrinfo(begin);  
        return off;
    }


    /*static*/ inet_address inet_address::localhost(inet_family family) {
        if (family == inet_family::IPV4) {
            uint8_t ip[] {127, 0, 0, 1};
            return inet_address::as_IPv4(ip, sizeof(ip));
        } 
        
        else if (family == inet_family::IPV6) {
            uint16_t ip[] {0, 0, 0, 0, 0, 0, 0, 1};
            return inet_address::as_IPv6(ip, sizeof(ip));
        } 
        
        else {
            throw_except<illegal_argument_exception>("Family %i is not family address", (int) family);
        }
        
        //Этот код недостижимый, но это дерьмо нужно, чтобы компилятор не визжал о том, что нет возвращаемого значения.
        return inet_address();
    }


    socket_address::socket_address() : m_address(), m_port(0) {

    }

    socket_address::socket_address(const inet_address& address, int32_t port) : m_address(address), m_port(port) {

    }

    socket_address::socket_address(const socket_address& sa) : m_address(sa.m_address), m_port(sa.m_port) {

    }

    socket_address::socket_address(socket_address&& sa) : m_address(std::move(sa.m_address)), m_port(sa.m_port) {

    }

    socket_address& socket_address::operator= (const socket_address& sa) {
        if (&sa != this) {
            m_address    = sa.m_address;
            m_port       = sa.m_port;
        }
        return *this;
    }
    
    socket_address& socket_address::operator= (socket_address&& sa) {
        if (&sa != this) {
            m_address    = std::move(sa.m_address);
            m_port       = sa.m_port;
        }
        return *this;
    }

    const inet_address& socket_address::get_address() const {
        return m_address;
    }
    
    int32_t socket_address::get_port() const {
        return m_port;
    }

    int64_t socket_address::hashcode() const {
        int64_t hash = m_address.hashcode();
        hash = hash + 233 * m_port;
        return hash;
    }
    
    bool socket_address::equals(const socket_address& sock_addr) const {
        if (&sock_addr == this)
            return true;
        return m_port == sock_addr.m_port && m_address.equals(sock_addr.m_address);
    }

    int socket_address::to_string(char buf[], int bufsize) {
        int off = m_address.to_string(buf, bufsize);
        if (bufsize - off > 0)
            off += snprintf(buf + off, bufsize - off, " port: %i", m_port);
        return off;
    }
    
}