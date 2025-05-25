#ifndef _JSTD_CPP_LANG_BASE_SOCKET_H_
#define _JSTD_CPP_LANG_BASE_SOCKET_H_
#include <cpp/lang/net/inetaddr.hpp>
#include <cpp/lang/net/socket_option.hpp>
#include <cstdint>

namespace jstd {

/**
 * @brief Абстрактный базовый класс для работы с сокетами.
 * 
 * @tparam 
 *      SOCKET_T Тип дескриптора сокета.
 * 
 * Этот класс предоставляет интерфейс для работы с сокетами, включая подключение,
 * приём и отправку данных, управление параметрами сокета и обработку соединений.
 * Класс не поддерживает копирование, но допускает перемещение.
 */
template<typename SOCKET_T>
class base_socket {
    base_socket(const base_socket&) = delete;
    base_socket& operator= (const base_socket&) = delete;
protected:
    //Значение для нулевого порта.    
    static const int32_t NULL_PORT = 0;

    //Дескриптор сокета.
    SOCKET_T        _ID;         
    
    // Адрес сокета.
    inet_address    _address;    
    
    // Порт сокета.
    int32_t         _port; 
    
    // Локальный порт сокета.
    int32_t         _localport;

    /**
     * @brief Конструктор по умолчанию.
     */
    base_socket();

    /**
     * @brief Конструктор перемещения.
     * 
     * @param other 
     *      Перемещаемый объект.
     */
    base_socket(base_socket&& other);

    /**
     * @brief 
     *      Оператор присваивания (перемещение).
     * 
     * @param other 
     *      Перемещаемый объект.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    base_socket& operator= (base_socket&& other);

    /**
     * @brief Виртуальный деструктор, обязательный для абстрактного класса.
     */
    virtual ~base_socket() = 0;

    /**
     * @brief Устанавливает параметр сокета.
     * 
     * @param opt_id 
     *      Идентификатор параметра.
     * 
     * @param value 
     *      Значение параметра.
     */
    virtual void set_socket_option(int32_t opt_id, const socket_option& value) = 0;

    /**
     * @brief Получает параметр сокета.
     * 
     * @param opt_id 
     *      Идентификатор параметра.
     * 
     * @return 
     *      Значение параметра.
     */
    virtual socket_option get_socket_option(int32_t opt_id) const = 0;

    /**
     * @brief Подключается к удалённому хосту.
     * 
     * @param host 
     *      Имя хоста или IP-адрес.
     * 
     * @param port 
     *      Порт хоста.
     * 
     * @throws io_exception
     *      Если произошла ошибка при подключении.
     */
    virtual void connect(const char* host, int32_t port) = 0;

    /**
     * @brief Подключается к удалённому хосту по объекту inet_address.
     * 
     * @param host 
     *      Адрес хоста.
     * @param port 
     *      Порт хоста.
     * 
     * @throws io_exception
     *      Если произошла ошибка при подключении.
     */
    virtual void connect(const inet_address& host, int32_t port) = 0;

    /**
     * @brief Привязывает сокет к указанному адресу и порту.
     * 
     * @param address 
     *      Адрес, к которому будет привязан сокет.
     * 
     * @param port 
     *      Порт для привязки.
     * 
     * @throws io_exception
     *      Если произошла ошибка при связывании сокета.
     */
    virtual void bind(const inet_address& address, int32_t port) = 0;

    /**
     * @brief Переводит сокет в режим прослушивания входящих подключений.
     * 
     * @param backlog 
     *      Размер очереди ожидающих подключений.
     * 
     * @throws io_exception
     *      Если произошла ошибка при изменении очереди ожидающих подключения.
     */
    virtual void listen(int32_t backlog) = 0;

    /**
     * @brief Принимает входящее соединение.
     * 
     * @param client 
     *      Указатель на новый клиентский сокет.
     * 
     * @return 
     *      true, если подключение успешно принято, иначе false.
     * 
     * @throws io_exception
     *      Если произошла ошибка при принятии сокета.
     */
    virtual bool accept(base_socket<SOCKET_T>* client) = 0;

    /**
     * @brief Закрывает сокет.
     * 
     * @throws io_exception
     *      Если произошла ошибка при закрытии сокета.
     */
    virtual void close() = 0;

    /**
     * @brief Останавливает приём данных на сокет.
     * 
     * @throws io_exception
     *      Если произошла ошибка при остановке приёма данных.
     */
    virtual void shutdown_in() = 0;

    /**
     * @brief Останавливает отправку данных через сокет.
     * 
     * @throws io_exception
     *      Если произошла ошибка при остановке отправки данных.
     */
    virtual void shutdown_out() = 0;

    /**
     * @brief Преобразует информацию о сокете в строку.
     * 
     * @param buf 
     *      Буфер для записи строки.
     * 
     * @param bufsize 
     *      Размер буфера.
     * 
     * @return 
     *      Длина записанной строки.
     */
    virtual int32_t to_string(char buf[], int32_t bufsize) const;

    /**
     * @brief Читает данные из сокета.
     * 
     * @param buf 
     *      Буфер для хранения прочитанных данных.
     * 
     * @param len 
     *      Максимальный размер данных для чтения.
     * 
     * @return 
     *      Количество прочитанных байт.
     * 
     * @throws io_exception
     *      Если произошла ошибка при чтении данных.
     */
    virtual int64_t read(char buf[], int64_t len) = 0;

    /**
     * @brief Отправляет данные через сокет.
     * 
     * @param data 
     *      Указатель на отправляемые данные.
     * 
     * @param len 
     *      Размер отправляемых данных в байтах.
     * 
     * @return 
     *      Количество отправленных байт.
     * 
     * @throws io_exception
     *      Если произошла ошибка при отправке данных.
     */
    virtual int64_t write(const char* data, int64_t len) = 0;

    /**
     * @brief Устанавливает режим блокировки сокета.
     * 
     * @param block 
     *      true — блокирующий режим, false — неблокирующий.
     * 
     * @throws io_exception
     *      Если произошла ошибка при установке режима сокета.
     */
    virtual void set_blocking(bool block) = 0;

    /**
     * @brief Проверяет, является ли сокет блокирующим.
     * 
     * @return 
     *      true, если сокет работает в блокирующем режиме, иначе false.
     */
    virtual bool is_blocking() const = 0;

public:
    /**
     * @brief Создаёт сокет указанного семейства.
     * 
     * @param family 
     *      Семейство адресов (например, AF_INET или AF_INET6).
     * 
     * @throws io_exception
     *      Если произошла ошибка при создании сокета.
     */
    virtual void create(inet_family family) = 0;

    /**
     * @brief Проверяет, был ли сокет создан.
     * 
     * @return 
     *      true, если сокет создан, иначе false.
     */
    virtual bool is_created() const = 0;

    /**
     * @brief Получает дескриптор сокета.
     * 
     * @return 
     *      Ссылка на дескриптор сокета.
     */
    SOCKET_T& get_descriptor();

    /**
     * @brief Получает дескриптор сокета (константная версия).
     * 
     * @return 
     *      Константная ссылка на дескриптор сокета.
     */
    const SOCKET_T& get_descriptor() const;

    /**
     * @brief Получает адрес сокета.
     * 
     * @return 
     *      Ссылка на объект inet_address.
     */
    inet_address& get_address();

    /**
     * @brief Получает адрес сокета (константная версия).
     * 
     * @return 
     *      Константная ссылка на объект inet_address.
     */
    const inet_address& get_address() const;

    /**
     * @brief Получает порт сокета.
     */
    int32_t get_port() const;

    /**
     * @brief Получает локальный порт сокета.
     */
    int32_t get_localport() const;

    /**
     * @brief Устанавливает адрес сокета.
     */
    void set_address(const inet_address& addr);

    /**
     * @brief Устанавливает порт сокета.
     */
    void set_port(int32_t port);

    /**
     * @brief Устанавливает локальный порт сокета.
     */
    void set_localport(int32_t localport);

    /**
     * @brief Устанавливает дескриптор сокета.
     */
    void set_descriptor(SOCKET_T&& sID);
};


    template<typename SOCKET_T>
    base_socket<SOCKET_T>::base_socket() : _ID(), _address(), _port(0), _localport(0) {

    }

    template<typename SOCKET_T>
    base_socket<SOCKET_T>::base_socket(base_socket<SOCKET_T>&& sock) : 
    _ID(std::move(sock._ID)), 
    _address(std::move(sock._address)), 
    _port(sock._port), 
    _localport(sock._localport) {
        sock._port      = NULL_PORT;
        sock._localport = NULL_PORT;
    }

    template<typename SOCKET_T>
    base_socket<SOCKET_T>& base_socket<SOCKET_T>::operator= (base_socket<SOCKET_T>&& sock) {
        if (&sock != this) {
            _ID         = std::move(sock._ID);
            _address    = std::move(sock._address);
            _port       = sock._port;
            _localport  = sock._localport;
            sock._port      = NULL_PORT;
            sock._localport = NULL_PORT;
        }
        return *this;
    }

    template<typename SOCKET_T>
    base_socket<SOCKET_T>::~base_socket() {
        
    }

    template<typename SOCKET_T>
    const SOCKET_T& base_socket<SOCKET_T>::get_descriptor() const {
        return _ID;
    }

    template<typename SOCKET_T>
    SOCKET_T& base_socket<SOCKET_T>::get_descriptor() {
        return _ID;
    }

    template<typename SOCKET_T>
    inet_address& base_socket<SOCKET_T>::get_address() {
        return _address;
    }

    template<typename SOCKET_T>
    const inet_address& base_socket<SOCKET_T>::get_address() const {
        return _address;
    }

    template<typename SOCKET_T>
    int32_t base_socket<SOCKET_T>::get_port() const {
        return _port;
    }
    
    template<typename SOCKET_T>
    int32_t base_socket<SOCKET_T>::get_localport() const {
        return _localport;
    }

    template<typename SOCKET_T>
    void base_socket<SOCKET_T>::set_address(const inet_address& addr) {
        _address = addr;
    }
    
    template<typename SOCKET_T>
    void base_socket<SOCKET_T>::set_port(int32_t port) {
        _port = port;
    }
    
    template<typename SOCKET_T>
    void base_socket<SOCKET_T>::set_localport(int32_t localport) {
        _localport = localport;
    }
    
    template<typename SOCKET_T>
    void base_socket<SOCKET_T>::set_descriptor(SOCKET_T&& sID) {
        _ID = std::move(sID);
    }
    
    template<typename SOCKET_T>
    int32_t base_socket<SOCKET_T>::to_string(char buf[], int32_t bufsize) const {
        char _ipbuf[64];
        _address.to_string(_ipbuf, sizeof(_ipbuf));
        return std::snprintf(buf, bufsize, "[addr=/%s, port=%i, localport=%i]", _ipbuf, (int) _port, _localport);
    }
}
#endif//_JSTD_CPP_LANG_BASE_SOCKET_H_