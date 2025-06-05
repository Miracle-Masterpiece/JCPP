#ifndef CPP_LANG_IO_INFLATER_INPUT_STREAM_H
#define CPP_LANG_IO_INFLATER_INPUT_STREAM_H

#include <allocators/base_allocator.hpp>
#include <cpp/lang/io/istream.hpp>
#include <cpp/lang/zip/inflater.hpp>
#include <cpp/lang/array.hpp>

namespace jstd {

class inflstream : public istream {
    static const int64_t DEFAULT_BUFFER_SIZE = 1024;
    
    array<char> m_buffer;
    istream*    m_in;
    inflater    m_inf;

    void fill();

public:
    using istream::read;
    
    inflstream();

    inflstream(istream* in, tca::base_allocator* allocator = tca::get_scoped_or_default(), int64_t buf_size = DEFAULT_BUFFER_SIZE);

    /**
     * Читает несколько байт из потока.
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
     *      Если произошла ошибка ввода/вывода
     */
    int64_t read(char buf[], int64_t sz) override;

    /**
     * Указывает, можно ли считать ещё из потока.
     * Если поток is_finished(), возвращется 0, иначе 1
     */
    int64_t available() const override;

    /**
     * Просто вызывает this->close(), но игнорирует любые исключения.
     * Для обработки исключений нужно явно вызывать this->close();
     */
    ~inflstream();

    /**
     * @brief Закрывает поток.
     * @throws io_exception Если произошла ошибка ввода/вывода
     */
    void close() override;
};


}

#endif//CPP_LANG_IO_INFLATER_INPUT_STREAM_H