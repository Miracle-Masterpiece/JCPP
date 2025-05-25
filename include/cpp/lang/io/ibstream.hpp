#ifndef _JSTD_CPP_LANG_IO_BUFFEREDINPUTSTREAM_H_
#define _JSTD_CPP_LANG_IO_BUFFEREDINPUTSTREAM_H_

#include <cpp/lang/io/istream.hpp>
#include <allocators/base_allocator.hpp>

namespace jstd {

/**
 * Класс предназначен для буферезированного чтения данных.
 * 
 * @remark
 *      Копирование и присваивание запрещены, чтобы избежать неожиданных побочных эффектов.  
 */
class ibstream : public istream {
    static const int DEFAULT_BUFFER_SIZE = 4096;
    tca::base_allocator*    _allocator;
    char*                   _buffer;
    int64_t                 _capacity;
    int64_t                 _offset;
    int64_t                 _limit;
    istream*                _in;

    //особождает память, выделенную под буфер.
    //если аллокатор != nullptr.
    //если аллокатор == nullptr - значит использовался внешний буфер.
    void free();
public:
    
    /**
     * Создаёт пустой, не открытый, поток.
     */
    ibstream();
    
    /**
     * @param stream
     *          Поток из которого будут читаться данные.
     * 
     * @param allocator
     *          Распределитель памяти, который владеет внутренним буфером.
     * 
     * @param buf_size
     *          Необязательный параметр, задающий размер внутреннего буфера.
     */
    ibstream(istream* stream, tca::base_allocator* allocator, int64_t buf_size = DEFAULT_BUFFER_SIZE);
    
    /**
     * @param stream
     *          Поток из которого будут читаться данные.
     * 
     * @param buf
     *          Буфер, который будет использоваться в качестве внутреннего буфеара.
     * 
     * @param buf_size
     *          Размер буфера.
     */
    ibstream(istream* stream, char* buf, int64_t buf_size);

    /**
     * Перемещение
     */
    ibstream(ibstream&& stream);

    /**
     * Перемещение
     * @throws io_exception Если произошла ошибка ввода/вывода
     */
    ibstream& operator= (ibstream&& stream);

    /**
     * @brief Читает один байт из потока.
     * @return Значение прочитанного байта (0–255) или -1, если достигнут конец потока.
     * @throws io_exception Если произошла ошибка ввода/вывода
     *                      Если поток не был открыт.
     */
    int read();

    /**
     * @brief Читает несколько байт из потока.
     * 
     * Заполняет переданный буфер данными из потока, но не более чем sz байт.
     * @param buf Указатель на буфер, в который будут записаны данные.
     * @param sz Максимальное количество байт для чтения.
     * @return Количество реально прочитанных байт или -1, если достигнут конец потока.
     * 
     * @throws io_exception Если произошла ошибка ввода/вывода
     *                      Если поток не был открыт.
     */
    int64_t read(char buf[], int64_t sz);

    /**
     * @brief Пропускает указанное количество байт во входном потоке.
     * 
     * Позволяет пропустить ненужные данные, не загружая их в память.
     * @param n Количество байт для пропуска.
     * @return Количество фактически пропущенных байт.
     * 
     * @throws io_exception Если произошла ошибка ввода/вывода
     *                      Если поток не был открыт.
     */
    int64_t skip(int64_t n = 1);

    /**
     * @brief Возвращает количество доступных для чтения байт.
     * 
     * Позволяет узнать, сколько данных можно прочитать без блокировки.
     * @return Количество доступных байт в потоке.
     * 
     * @throws io_exception Если произошла ошибка ввода/вывода
     *                      Если поток не был открыт.
     */
    int64_t available() const;

    /**
     * Если не был вызван метод this->close(), 
     * то вызывает его, но игнорирует любые исключения.
     */
    ~ibstream();

    /**
     * @brief Закрывает поток файла.
     * @throws io_exception Если произошла ошибка ввода/вывода
     *                      Если поток не был открыт.
     *                      Если поток уже закрыт.
     */
    void close();

};

}
#endif//_JSTD_CPP_LANG_IO_BUFFEREDINPUTSTREAM_H_