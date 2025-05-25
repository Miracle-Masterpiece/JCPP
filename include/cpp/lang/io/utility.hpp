#ifndef _JSTD_CPP_LANG_IO_IOUTILITY_H_
#define _JSTD_CPP_LANG_IO_IOUTILITY_H_
#include <cpp/lang/io/constants.hpp>

namespace jstd {

/**
 * file_filter — базовый класс для фильтрации файлов.
 */
class file_filter {
public:
    file_filter() = default;
    
    /**
     * Проверяет, подходит ли файл под фильтр.
     * 
     * @param path 
     *      Путь к файлу (UTF-8).
     * 
     * @param path_length 
     *      Длина пути.
     * 
     * @return 
     *      true, если файл подходит.
     */
    virtual bool apply(const char* path, int path_length) const = 0;

    // Уничтожает фильтр.
    virtual ~file_filter() {};
};
    
//Фильтр, который принимает всё.  
class accept_all_filter : public file_filter {
public:
    // Всегда возвращает true.     
    bool apply(const char* path, int path_length) const {return true;} 
};

}
#endif//_JSTD_CPP_LANG_IO_IOUTILITY_H_