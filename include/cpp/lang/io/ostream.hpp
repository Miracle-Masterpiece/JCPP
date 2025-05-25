#ifndef _JSTD_CPP_LANG_IO_OUTPUTSTREAM_H_
#define _JSTD_CPP_LANG_IO_OUTPUTSTREAM_H_
#include <cstdint>
namespace jstd{

/**
 * @brief Абстрактный класс для работы с выходными потоками.
 *
 * Обеспечивает интерфейс для записи данных в различные выходные потоки, 
 * такие как файлы, буферы памяти или сетевые соединения.
 *
 * @note Копирование запрещено.
 */
class ostream {
    ostream(const ostream&)               = delete; ///< Запрещено копирование.
    ostream& operator= (const ostream&)   = delete; ///< Запрещено присваивание.
public:
    /**
     * @brief Конструктор по умолчанию.
     */
    ostream() {}
    
    /**
     * @brief Записывает один символ в поток.
     * 
     * @param c Символ для записи.
     * 
     * @throws io_exception Если произошла ошибка ввода/вывода
     */
    virtual void write(char c);

    /**
     * @brief Записывает массив байтов в поток.
     * 
     * @param data Указатель на массив байтов.
     * 
     * @throws io_exception Если произошла ошибка ввода/вывода
     */
    virtual void write(const char* data, int64_t sz) = 0;

    /**
     * @brief Сбрасывает буферизированные данные.
     * 
     * @throws io_exception Если произошла ошибка ввода/вывода
     */
    virtual void flush() = 0;

    /**
     * @brief Закрывает поток.
     * 
     * @throws io_exception Если произошла ошибка ввода/вывода
     */
    virtual void close() = 0;

    /**
     * @brief Деструктор.
     */
    virtual ~ostream();
};

}
#endif//_JSTD_CPP_LANG_IO_OUTPUTSTREAM_H_

