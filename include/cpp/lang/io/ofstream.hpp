#ifndef _JSTD_CPP_LANG_IO_FILEOUTPUTSTREAM_H_
#define _JSTD_CPP_LANG_IO_FILEOUTPUTSTREAM_H_

#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/io/file.hpp>
#include <cstdio>

namespace jstd {

/**
 * Класс предназначен для записи в файл.
 * 
 * @remark
 *      Копирование и присваивание запрещены, чтобы избежать неожиданных побочных эффектов.  
 */
class ofstream : public ostream {
    FILE* _handle;  //указатель на файл.
public:
    /**
     * Создаёт пустой файловый поток.
     */    
    ofstream();
    
    /**
     * Создаёт поток по указанному пути.
     * 
     * @param path  Путь к файлу.
     * 
     * @param append  Режим открытия.
     *              true - добавляет новые данный в файл.
     *              false - стирает старые данные из файла и записывает сначала.
     * 
     * @param path_length   Необязательный параметр, обозначающий длину строки path. 
     *                      Если значение -1, длина строки вычислится автоматически, но строка должна оканчиваться на \0.
     * 
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода.
     * 
     * @throws sequrity_exception 
     *          Если запись в файл запрещено.
     */
    ofstream(const char* path, bool append = false, int path_length = -1);

    /**
     * Создаёт поток по объекту файла.
     * 
     * @param file  
     *      Объект, описыващий файл.
     * 
     * @param append
     *      Режим открытия.
     *      true - добавляет новые данный в файл.
     *      false - стирает старые данные из файла и записывает сначала.
     * 
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода.
     * 
     * @throws sequrity_exception 
     *          Если запись в файл запрещено.
     */
    ofstream(const file& f, bool append = false);
    
    //Перемещение.
    ofstream(ofstream&&);
    
    /**
     * Перемещение.
     * 
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода.
     */
    ofstream& operator= (ofstream&&);
    
    /**
     * @note
     *      Для освобождения ресурсов должен явно вызываться this::close()
     */
    ~ofstream();
    
    /**
     * Записывайт байт в поток.
     * 
     * @param c 
     *      Значение байта, которое будет записано в поток.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     */
    void write(char c);
    
    /**
     * Записывает sz байт в потока из передаваемого буфера.
     * 
     * @param data   
     *      Буфер из которого будут записаны данные.
     * 
     * @param sz     
     *      Сколько байт нужно записать в поток.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     */
    void write(const char* data, int64_t sz);
    
    /**
     * Сбрасывает данные из потока.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     */
    void flush();
    
    /**
     * Закрывает поток и сбрасывает данные.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     */
    void close();
};

} 
#endif//_JSTD_CPP_LANG_IO_FILEOUTPUTSTREAM_H_
