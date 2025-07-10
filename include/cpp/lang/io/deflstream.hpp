#ifndef CPP_LANG_IO_DEFLATER_OUTPUT_STREAM_H
#define CPP_LANG_IO_DEFLATER_OUTPUT_STREAM_H

#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/array.hpp>
#include <cpp/lang/zip/deflater.hpp>

namespace jstd {

/**
 * Выходной поток для сжатых данных.
 * 
 * @note
 *      Для закрытия ресурсов необходимо вызывать функцию deflstream::close().
 *      Деструктор не освобождает данные! В силу требований об явной обработке ошибок.
 */
class deflstream : public ostream {
    static const int32_t DEFAULT_BUFFER_SIZE = 1024;

    /**
     * Временный буфер для сжимаемых данных.
     */
    array<char> m_buffer;
    
    /**
     * Поток, куда будут записываться сжимаемые данные.
     */
    ostream*    m_out;

    /**
     * Сжиматель данных
     */
    deflater    m_def;

public:
    /**
     * Конструктор по умолчанию.
     */
    deflstream();

    /**
     * Конструктор для инициализации потока сжатия.
     * 
     * @param out
     *      Поток, куда будут направляться сжатые данные.
     * 
     * @param allocator
     *      Распределитель памяти для выделения буфера для сжимаемых данных.
     * 
     * @param buf_size
     *      Размер буфера для сжимаемых данных.
     */
    deflstream(ostream* out, tca::base_allocator* allocator = tca::get_scoped_or_default(), int64_t buf_size = DEFAULT_BUFFER_SIZE);
    
    /**
     *  Констуктор для инициализации потока сжатия с внешним буфером.
     * 
     * @param out
     *      Поток, куда будут направляться сжатые данные.
     * 
     * @param buffer
     *      Буфер для сжимаемых данных.
     * 
     * @param buf_size
     *      Размер буфера для сжимаемых данных.
     */
    deflstream(ostream* out, char* buffer, int64_t buf_size);

    /**
     * Конструктор перемещения.
     */
    deflstream(deflstream&&);
    
    /**
     * Оператор перемещения
     */
    deflstream& operator= (deflstream&&);

    /**
     * Записывает массив байтов в поток.
     * 
     * @param data 
     *      Указатель на массив байтов.
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

    /**
     * 
     */
    void finish();

    /**
     * @note
     *      Для освобождения ресурсов должен явно вызываться this::close()
     */
    ~deflstream();  
};

}

#endif//CPP_LANG_IO_DEFLATER_OUTPUT_STREAM_H