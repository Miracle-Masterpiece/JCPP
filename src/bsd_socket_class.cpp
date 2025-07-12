#include <internal/bsd_socket_class.hpp>
#include <internal/bsd_socket.hpp>
#include <iostream>

#define NULL_SOCK -1

namespace jstd 
{
namespace bsd_socket 
{


    sock_id::sock_id() : ID(NULL_SOCK) {

    }
    
    sock_id::sock_id(int32_t ID) : ID(ID) {

    }
    
    sock_id::sock_id(sock_id&& id) : ID(id.ID) {
        id.ID   = NULL_SOCK;
    }
    
    sock_id& sock_id::operator= (sock_id&& id) {
        if (&id != this) {
            if (ID != NULL_SOCK)
                close();
            ID      = id.ID;
            id.ID   = NULL_SOCK;
        }
        return *this;
    }

    sock_id::~sock_id() {

    }

    void sock_id::close() {
        if (ID == NULL_SOCK)
            return;
        try {
            bsd_socket::close(ID);
            ID = NULL_SOCK;
        } catch (...) {
            ID = NULL_SOCK;
            throw;
        }
    }

/**
 * =====================================================================================================================================================================
 * =====================================================================================================================================================================
 */

    socket_impl::socket_impl() : base_socket<sock_id>(), _blocking(true) {
        
    }
    
    socket_impl::socket_impl(socket_impl&& sock) : base_socket<sock_id>(std::move(sock)), _blocking(sock._blocking) {

    }

    socket_impl& socket_impl::operator= (socket_impl&& sock) {
        base_socket<sock_id>::operator=(std::move(sock));
        _blocking = sock._blocking;
        return *this;
    }

    void socket_impl::set_socket_option(int32_t opt_id, const socket_option& value) {
        bsd_socket::set_sock_opt(_ID.ID, opt_id, value);
    }
    
    socket_option socket_impl::get_socket_option(int32_t opt_id) const {
        socket_option ret;
        bsd_socket::get_sock_opt(_ID.ID, opt_id, ret);
        return ret;
    }
    
    void socket_impl::connect(const char* host, int32_t port) {
        connect(inet_address(host), port);
    }

    void socket_impl::connect(const inet_address& host, int32_t port) {
        bsd_socket::connect(_ID.ID, host, port);
        socket_address addr;
        bsd_socket::get_sock_name(_ID.ID, addr);
        set_port(port);
        set_address(host);
        set_localport(addr.get_port());
    }
    
    void socket_impl::bind(const inet_address& address, int32_t port) {
        bsd_socket::bind(_ID.ID, address, port);
    }

    void socket_impl::listen(int32_t backlog) {
        bsd_socket::listen(_ID.ID, backlog);
    }

    bool socket_impl::accept(base_socket<sock_id>* client) {
        if (client == nullptr)
            throw_except<null_pointer_exception>("client is nullptr!");
        socket_address client_address;
        int32_t client_sock_id = bsd_socket::accept(_ID.ID, &client_address);
        if (client_sock_id == -1)
            return false;
        client->set_address(client_address.get_address());
        client->set_localport(_port);
        client->set_port(client_address.get_port());
        client->set_descriptor(sock_id(client_sock_id));
        return true;
    }
    
    void socket_impl::create(inet_family family) {
        sock_id sockID(bsd_socket::open_tcp(family));
        _ID = std::move(sockID);        
    }
    
    bool socket_impl::is_created() const {
        return _ID.ID == NULL_SOCK;
    }

    void socket_impl::close() {
        _ID = sock_id(NULL_SOCK);
    }
    
    void socket_impl::shutdown_in() {
        bsd_socket::shutdown_in(_ID.ID);
    }
    
    void socket_impl::shutdown_out() {
        bsd_socket::shutdown_out(_ID.ID);   
    }
    
    int64_t socket_impl::read(char buf[], int64_t len) {
        return bsd_socket::recv(_ID.ID, buf, len, _blocking);
    }
    
    int64_t socket_impl::write(const char* data, int64_t len) {
        return bsd_socket::send(_ID.ID, data, len, _blocking);
    }

    void socket_impl::set_blocking(bool is_block) {
        bsd_socket::set_blocking(_ID.ID, is_block);
        _blocking = is_block;
    }
    
    bool socket_impl::is_blocking() const {
        return _blocking;
    }

    socket_impl::~socket_impl() {
        
    }

}

}