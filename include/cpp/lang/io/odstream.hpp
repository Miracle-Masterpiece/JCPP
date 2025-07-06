#ifndef _JSTD_CPP_LANG_IO_DATA_OUTPUT_STREAM_H_
#define _JSTD_CPP_LANG_IO_DATA_OUTPUT_STREAM_H_

#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/system.hpp>
#include <cpp/lang/utils/utils.hpp>

namespace jstd {

/**
 * Класс предназначен для буферезированной записи бинарных данных.
 * Бинарные данные будут записаны в порядке байт Little-Endian, даже на Big-Endian платформе. 
 * 
 * @remark
 *      Копирование и присваивание запрещены, чтобы избежать неожиданных побочных эффектов.  
 */
class odstream : public ostream {
    ostream* _out;
public:
    using ostream::write;
    
    /**
     * Создаёт пустой поток.
     */
    odstream();
    
    /**
     * Оборачивает поток вывода, для записи в него бинарных данных.
     * @throws null_pointer_exception
     *          Если out равен nullptr.
     */
    odstream(ostream* out);
    
    //Перемещение
    odstream(odstream&&);
    
    //Перемещение
    /**
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода.
     */
    odstream& operator= (odstream&&);
    
    /**
     * Вызывает метод this->close(), но игнорирует исключения.
     */
    ~odstream();

    /**
     * @brief Записывает массив байтов в поток.
     * 
     * @param data Указатель на массив байтов.
     * 
     * @throws io_exception Если произошла ошибка ввода/вывода
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода
     *          Если поток закрыт или не был открыт
     */
    void write(const char* data, int64_t sz);
    
    /**
     * @brief Сбрасывает буферизированные данные.
     * 
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода
     *          Если поток закрыт или не был открыт
     */
    void flush();

    /**
     * @brief Закрывает поток.
     * 
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода
     *          Если поток закрыт или не был открыт
     */
    void close();
    
    /**
     * @see this->write<T>(const T* v, int64_t sz);
     */
    template<typename T>
    inline void write(T v) {
        write<T>(&v, 1);
    }

    /**
     * Записывает в поток массив типа T в порядке байт Little-Endian.
     * @tparam 
     *      Любой тип, который тривиально-копируемый
     * 
     * @remark 
     *      Данные записываются в порядке байт Little-Endian.
     * 
     * @param v
     *      Указатель на массив данных типа T.
     * 
     * @param sz
     *      Размер массива v
     */
    template<typename T>
    void write(const T* v, int64_t sz);
};

    template<typename T>
    void odstream::write(const T* v, int64_t sz) {
        if (system::native_byte_order() != byte_order::LE) {
            T tmp;
            for (int64_t i = 0; i < sz; ++i) {
                tmp = utils::bswap<T>(v[i]);
                write(reinterpret_cast<const char*>(&tmp), sizeof(T));
            }
        } 
        else {
            write(reinterpret_cast<const char*>(v), sizeof(T) * sz);
        }
    }
}

#endif//_JSTD_CPP_LANG_IO_DATA_OUTPUT_STREAM_H_