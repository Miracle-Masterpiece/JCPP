#ifndef _JSTD_CPP_LANG_IO_DATA_INPUT_STREAM_H_
#define _JSTD_CPP_LANG_IO_DATA_INPUT_STREAM_H_
#include <cpp/lang/io/istream.hpp>
#include <cpp/lang/system.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/utils.hpp>

namespace jstd {

/**
 * Класс предназначен для буферезированного чтения бинарных данных.
 * Бинарные данные будут прочтены из порядка байт Little-Endian, даже на Big-Endian платформе, в системный порядок байт. 
 * 
 * @remark
 *      Копирование и присваивание запрещены, чтобы избежать неожиданных побочных эффектов.  
 */
class idstream : public istream {
    istream* _in;
public:
    using istream::read;

    /**
     * Создаёт пустой поток.
     */
    idstream();
    

    /**
     * Оборачивает поток ввода, для чтения из него бинарных данных.
     * 
     * @throws null_pointer_exception
     *          Если in равен nullptr.
     */
    idstream(istream* in);
    
    //Перемещение
    idstream(idstream&&);
    
    //Перемещение
    /**
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода.
     */
    idstream& operator= (idstream&&);

    /**
     * Если не был вызван метод this->close(), 
     * то вызывает его, но игнорирует любые исключения.
     */
    ~idstream();
    
    /**
     * @brief Читает несколько байт из потока.
     * 
     * Заполняет переданный буфер данными из потока, но не более чем sz байт.
     * @param buf Указатель на буфер, в который будут записаны данные.
     * @param sz Максимальное количество байт для чтения.
     * @return Количество реально прочитанных байт или -1, если достигнут конец потока.
     * @throws io_exception Если произошла ошибка ввода/вывода
     */
    int64_t read(char buf[], int64_t sz);

    /**
     * @brief Возвращает количество доступных для чтения байт.
     * 
     * Позволяет узнать, сколько данных можно прочитать из этого потока.
     * 
     * @return Количество доступных байт в потоке.
     * 
     * @throws io_exception Если произошла ошибка ввода/вывода
     */
    int64_t available() const;

    /**
     * @brief Закрывает поток файла.
     * @throws io_exception Если произошла ошибка ввода/вывода
     */
    void close();

    /**
     * @see this->read<T>(T buf[], int64_t sz);
     */
    template<typename T>
    T read() {
        T tmp;
        read<T>(&tmp, 1);
        return tmp;
    }

    /**
     * Читает из потока массив типа T в порядке байт Little-Endian.
     * @tparam 
     *      Любой тип, который тривиально-копируемый
     * 
     * @remark 
     *      Данные читаются из порядка Little-Endian.
     * 
     * @param buf
     *      Указатель на массив данных типа T, куда будут записаны данные.
     * 
     * @param sz
     *      Размер массива v
     * 
     * @throws io_exception
     *          Если произошла ошибка ввода/вывода.
     *      
     * @throws eof_exception
     *          Если попытка прочитать больше данных, чем есть в потоке.
     */
    template<typename T>
    int64_t read(T buf[], int64_t sz);
};

    template<typename T>
    int64_t idstream::read(T buf[], int64_t sz) {
        int64_t readed = read(reinterpret_cast<char*>(buf), sizeof(T) * sz);
        if (readed != (int64_t) sizeof(T) * sz)
            throw_except<eof_exception>("Cannot read type");   
        if (system::native_byte_order() != byte_order::LE) {
            for (int64_t i = 0; i < sz; ++i)
                utils::swap(reinterpret_cast<void*>(buf + i), sizeof(T));
        }
        return sz;
    }

}

#endif//_JSTD_CPP_LANG_IO_DATA_INPUT_STREAM_H_