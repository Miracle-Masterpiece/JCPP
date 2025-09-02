#ifndef _JSTD_INTERNAL_BSD_SOCKET_FUNCS_H_
#define _JSTD_INTERNAL_BSD_SOCKET_FUNCS_H_

#include <cstdint>
#include <cpp/lang/net/inetaddr.hpp>
#include <cpp/lang/net/socket_option.hpp>

namespace jstd
{

namespace bsd_socket
{
    
    /**
     * @throws socket_exception 
     *      Если произошла ошибка при открытие сокета.
     */
    int open_tcp(inet_family family);
    
    /**
     * @throws socket_exception 
     *      Если произошла ошибка при закрытии сокета.
     */
    void close(int32_t sock);

    /**
     * @throws bind_exception 
     *      Если произошла ошибка при привязке сокета.
     * 
     * @throws illegal_argument_exception
     *      Если тип семейства адреса не поддерживается.
     */
    void bind(int32_t sock, const inet_address& address, int32_t port);

    /**
     * @throws socket_exception 
     *      Если произошла ошибка при установки размера максимальной очереди.
     */
    void backlog(int32_t sock, int32_t maxq);

    /**
     * @throws socket_exception 
     *      Если произошла ошибка при установки размера максимальной очереди.
     */
    inline void listen(int32_t sock , int32_t maxq) {
        return backlog(sock, maxq);
    }

    /**
     * @throws socket_exception 
     *      Если произошла ошибка при приняти клиента.
     */
    int32_t accept(int32_t serv_sock, socket_address* client_addr);

    /**
     * @throws socket_exception 
     *      Если произошла ошибка при установке блокирующего/неблокирущего режима.
     */
    void set_blocking(int32_t sock, bool block_mode);

    /**
     * @throws connect_exception 
     *      Если произошла ошибка при подключении.
     * 
     * @throws illegal_argument_exception
     *      Если тип семейства адреса не поддерживается.
     */
    void connect(int32_t client_sock, const inet_address& address, int32_t port);

    /**
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     */
    void set_sock_opt(int32_t sock, int32_t opt, const socket_option& value);

    /**
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     */
    void get_sock_opt(int32_t sock, int32_t opt, socket_option& value);

    /**
     * @throws socket_exception
     *      Если произошла ошибка сокета.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода-вывода.
     * 
     * @throws socket_timeout_exception
     *      Если произошёл таймаут.
     * 
     * @throw connect_exception
     *      Если соединение было разорвано.
     * 
     * @return
     *      Сколько фактически отправилось байт.     
     */
    int64_t send(int32_t sock, const char* data, int64_t length, bool is_blocking);

    /**
     * @throws socket_exception
     *      Если произошла ошибка сокета.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода-вывода.
     * 
     * @throws socket_timeout_exception
     *      Если произошёл таймаут.
     * 
     * @throw connect_exception
     *      Если соединение было разорвано.
     * 
     * @return
     *      Сколько фактически прочиталось байт.
     */
    int64_t recv(int32_t sock, char* data, int64_t length, bool is_blocking);

    /**
     * @throws socket_exception
     *      Если произошла ошибка сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если имя сокета не поддерживает семейство адресов отличное от IPv4 и IPv6
     */
    void get_sock_name(int32_t sock, socket_address& address);

    /**
     * @throws socket_exception
     *      Если произошла ошибка при заглушении чтения.
     */
    void shutdown_in(int32_t sock);
    
    /**
     * @throws socket_exception
     *      Если произошла ошибка при заглушении записи.
     */
    void shutdown_out(int32_t sock);

}// namespace bsd_socket
}// namespace jstd

#endif//_JSTD_INTERNAL_BSD_SOCKET_FUNCS_H_