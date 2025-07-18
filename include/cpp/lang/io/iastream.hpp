#ifndef _JSTD_CPP_LANG_IO_ARRAY_INPUT_STREAM_H_
#define _JSTD_CPP_LANG_IO_ARRAY_INPUT_STREAM_H_

#include <cpp/lang/io/istream.hpp>
#include <allocators/base_allocator.hpp>

namespace jstd {

/**
 * Класс предназначен для представления байтового массива, как входного потока.
 * 
 * @note
 *      Для закрытия ресурсов необходимо вызывать функцию iastream::close().
 *      Деструктор не освобождает данные! В силу требований об явной обработке ошибок.
 */
class iastream : public istream {
    const char* _buffer;    
    int64_t     _capacity;
    int64_t     _offset;
public:
    /**
     * Пустой поток ввода.
     */
    iastream();
    
    /**
     * @param buffer
     *      Буфер, откуда будут читаться данные.
     * 
     * @param capacity
     *      Размер буфера, означающий сколько байт доступно для чтения.
     */
    iastream(const char* buffer, int64_t capacity);
    
    //перемещение
    iastream(iastream&&);
    
    //перемещение
    iastream& operator= (iastream&&);
    
    /**
     * @note
     *      Для освобождения ресурсов должен явно вызываться this::close()
     */
    ~iastream();
    
    /**
     * Читает один байт из потока.
     * 
     * @return 
     *      Значение прочитанного байта (0–255) или -1, если достигнут конец потока.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода
     *      Если поток не был открыт.
     *      Если поток уже закрыт.
     */
    int read();
    
    /**
     * Заполняет переданный буфер данными из потока, но не более чем sz байт.
     * 
     * @param buf 
     *      Указатель на буфер, в который будут записаны данные.
     * 
     * @param sz 
     *      Максимальное количество байт для чтения.
     * 
     * @return 
     *      Количество реально прочитанных байт или -1, если достигнут конец потока.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     *      Если поток уже закрыт.
     */
    int64_t read(char buf[], int64_t sz);
    
     /**
     * Возвращает количество доступных для чтения байт.
     * 
     * @return Количество доступных байт в потоке.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     *      Если поток уже закрыт.
     */
    int64_t available() const;
    
    /**
     * Закрывает поток.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     */
    void close();
    
    /**
     * Сбрасывает текущее смещение и начинает читать данные сначала.
     * Можно сказать, что переоткрывает поток.
     */
    void reset();
};


}

#endif//_JSTD_CPP_LANG_IO_ARRAY_INPUT_STREAM_H_