#ifndef _JSTD_CPP_LANG_IO_ARRAY_OUTPUT_STREAM_H_
#define _JSTD_CPP_LANG_IO_ARRAY_OUTPUT_STREAM_H_

#include <cpp/lang/io/ostream.hpp>
#include <allocators/base_allocator.hpp>

namespace jstd {

/**
 * Класс предназначен для представления байтового массива, как выходной поток в который сохраняются данные.
 * 
 * @note
 *      Для закрытия ресурсов необходимо вызывать функцию oastream::close().
 *      Деструктор не освобождает данные! В силу требований об явной обработке ошибок.
 */
class oastream : ostream {
    static const int INIT_BUF_SIZE = 16;
    tca::base_allocator* _allocator;    // - это может быть nullptr, тогда это означает, что this->_buffer является внешним массивом.
    char*   _buffer;
    int64_t _capacity;
    int64_t _offset;
    void resize(int64_t sz);
public:
    /**
     * @brief Конструктор по умолчанию.
     */
    oastream();
    
    /**
     * Инициализирует класс с помощью аллокатора и начальным размером буфера.
     * 
     * @param allocator
     *      Аллокатор, который управляет памятью.
     * 
     * @param init_buf_size
     *      Начальный размер буфера.
     * 
     * @remark
     *      Буфер можно быть выделен лениво, при первом использовании.
     */
    oastream(tca::base_allocator* allocator, int64_t init_buf_size = INIT_BUF_SIZE);

    /**
     * Инициализирует класс уже существующим массивом.
     * 
     * @param buf
     *      Массив, куда будут записываться данные.
     * 
     * @param capacity
     *      Размер массива, в который будут записываться данные.
     */
    oastream(char* buf, int64_t capacity);

    //перемещение
    oastream(oastream&&);

    //перемещение
    oastream& operator= (oastream&&);

    /**
     * @brief Записывает один символ в поток.
     * 
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода
     * 
     * @throws buffer_owerflow
     *          Если для класса используется внешний массив и места в нём не хватает.
     */
    void write(char c);

    /**
     * @brief Записывает массив байтов в поток.
     * 
     * @param data Указатель на массив байтов.
     * 
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода
     * 
     * @throws buffer_owerflow
     *          Если для класса используется внешний массив и места в нём не хватает.
     */
    void write(const char* data, int64_t sz);

    /**
     * Ничего не делает.
     */
    void flush();

    /**
     * Если this->_buffer не является внешним массивом, то освобождает память.
     * 
     * @throws io_exception
     *          Если поток не открыт.
     */
    void close();

    /**
     * @note
     *      Для освобождения ресурсов должен явно вызываться this::close()
     */
    ~oastream();

    /**
     * Возвращает указатель на внутренний буфер.
     */
    const char* data() const;

    /**
     * Возвращает размер полезных данных в потоке.
     */
    int64_t offset() const;
};

}

#endif//_JSTD_CPP_LANG_IO_ARRAY_OUTPUT_STREAM_H_