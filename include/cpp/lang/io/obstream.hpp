#ifndef _JSTD_CPP_LANG_IO_BUFFEREDOUTPUTSTREAM_H_
#define _JSTD_CPP_LANG_IO_BUFFEREDOUTPUTSTREAM_H_

#include <cpp/lang/io/ostream.hpp>
#include <allocators/base_allocator.hpp>

namespace jstd {

/**
 * Класс предназначен для буферезированной записи данных.
 * 
 * @note
 *      Для закрытия ресурсов необходимо вызывать функцию ibstream::close().
 *      Деструктор не освобождает данные! В силу требований об явной обработке ошибок.
 */
class obstream : public ostream {
    /**
     * 
     */
    static const int DEFAULT_BUFFER_SIZE = 0x1000;
    
    tca::base_allocator* _allocator;    /**/
    char*       _buffer;                /**/
    int64_t     _capacity;              /**/
    int64_t     _offset;                /**/
    ostream*    _out;                   /**/

    /**
     * @internal
     * Очищает память на которую указывает this->_buffer
     */
    void free();

public:
    /**
     * Конструктор по умолчанию.
     */
    obstream();
    
    /**
     * @param stream
     *      Поток в которого будут записываться данные.
     * 
     * @param allocator
     *      Распределитель памяти, который владеет внутренним буфером.
     * 
     * @param buf_size
     *      Необязательный параметр, задающий размер внутреннего буфера.
     * 
     * @throws null_pointer_exception
     *      Если stream == nullptr
     *      Если allocator == nullptr
     */
    obstream(ostream* stream, tca::base_allocator* allocator, int64_t buf_size = DEFAULT_BUFFER_SIZE);

    /**
     * @param stream
     *      Поток в который будут записываться данные.
     * 
     * @param buf
     *      Буфер, который будет использоваться в качестве внутреннего буфеара.
     * 
     * @param buf_size
     *      Размер буфера.
     * 
     * @throws null_pointer_exception
     *      Если stream == nullptr
     *      Если buffer == nullptr
     */
    obstream(ostream* stream, char* buffer, int64_t buf_size);

    /**
     * Перемещение.
     */
    obstream(obstream&& stream);

    /**
     * Перемещение.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода
     */
    obstream& operator= (obstream&& stream);

    /**
     * @note
     *      Для освобождения ресурсов должен явно вызываться this::close()
     */
    ~obstream();

    /**
     * Записывает один символ в поток.
     * 
     * @param c
     *      Символ для записи.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода
     */
    void write(char c);

    /**
     * Записывает массив байтов в поток.
     * 
     * @param data
     *      Указатель на массив байтов.
     * 
     * @param sz
     *      Количество байтов для записи.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода
     */
    void write(const char* data, int64_t sz);

    /**
     * Сбрасывает буферизированные данные.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода
     */
    void flush();

    /**
     * Закрывает поток.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода
     */
    void close();
};

}


#endif//_JSTD_CPP_LANG_IO_BUFFEREDOUTPUTSTREAM_H_