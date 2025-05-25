#ifndef _JSTD_INTERNAL_BSD_SOCKET_CLASS_H_
#define _JSTD_INTERNAL_BSD_SOCKET_CLASS_H_

#include <cpp/lang/net/base_socket.hpp>
#include <cstdint>

namespace jstd {
namespace bsd_socket {


struct sock_id {
    int32_t ID;
    sock_id();
    sock_id(int32_t ID);
    sock_id(sock_id&&);
    sock_id& operator= (sock_id&&);
    ~sock_id();
private:
    void close();
    sock_id(const sock_id&) = delete;
    sock_id& operator= (const sock_id&) = delete;
};

class socket_impl : public base_socket<sock_id> {
    /**
     * Является ли сокет неблокирующим.
     */
    bool _blocking;    
    
public:
    
    /**
     * Создаёт непривязанный сокет
     */
    socket_impl();
    
    /**
     * Перемещает данные из другого сокета в этот.
     */
    socket_impl(socket_impl&&);

    /**
     * Перемещает данные из одного сокета в этот.
     * Если этот сокет открыт, закрывает его.
     * 
     * @throws socket_exception
     *      Если при закрытие этого сокета произошла ошибка.
     * 
     * @return
     *      Этот сокет.
     */
    socket_impl& operator= (socket_impl&&);

    /**
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был открыт.
     *      Если сокет уже закрыт.
     */
    void set_socket_option(int32_t opt_id, const socket_option& value);
    
    /**
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был открыт.
     *      Если сокет уже закрыт.
     */
    socket_option get_socket_option(int32_t opt_id) const;
    
    /**
     * @throws connect_exception 
     *      Если произошла ошибка при подключении.
     */
    void connect(const char* host, int32_t port);
    
    /**
     * @throws connect_exception 
     *      Если произошла ошибка при подключении.
     * 
     * @throws illegal_argument_exception
     *      Если тип семейства адреса не поддерживается
     */
    void connect(const inet_address& host, int32_t port);
    
    /**
     * @throws bind_exception 
     *      Если произошла ошибка при привязке сокета.
     * 
     * @throws illegal_argument_exception
     *      Если тип семейства адреса не поддерживается.
     */
    void bind(const inet_address& address, int32_t port);
    
    /**
     * @throws socket_exception 
     *      Если произошла ошибка при установки размера максимальной очереди.
     */
    void listen(int32_t backlog);
    
    /**
     * @throws socket_exception
     *      Если произошла ошибка сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если имя сокета не поддерживает семейство адресов отличное от IPv4 и IPv6
     */
    bool accept(base_socket<sock_id>* client);
    
    /**
     * Создаёт дескриптор сокета для указанного семейства адресов.
     */
    void create(inet_family family);
    
    /**
     * Сознал ли дескриптор сокета.
     */
    bool is_created() const;

    /**
     * @throws socket_exception 
     *      Если произошла ошибка при закрытии сокета.
     */
    void close();
    
    /**
     * @throws socket_exception
     *      Если произошла ошибка при заглушении чтения.
     */
    void shutdown_in();
    
    /**
     * @throws socket_exception
     *      Если произошла ошибка при заглушении записи.
     */
    void shutdown_out();

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
    int64_t read(char buf[], int64_t len);
    
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
    int64_t write(const char* data, int64_t len);

    /**
     * Закрывает сокет и подавляет любые исключения.
     * Чтобы обработать исключения, вызывайте close() явно.
     */
    ~socket_impl();

    /**
     * @throws socket_exception 
     *      Если произошла ошибка при установке блокирующего/неблокирущего режима.
     */
    void set_blocking(bool non_block);
    
    /**
     * Является ли сокет блокирующим.
     * @return true - если блокирующий, false - неблокирующий.
     */
    bool is_blocking() const;
};

}//bsd_socket
}//jstd
#endif//_JSTD_INTERNAL_BSD_SOCKET_CLASS_H_