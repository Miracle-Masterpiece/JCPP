#ifndef _JSTD_CPP_LANG_IO_FILE_H_
#define _JSTD_CPP_LANG_IO_FILE_H_
#include <cpp/lang/array.hpp>
#include <cstdint>

namespace tca{
    class base_allocator;
}

namespace jstd {

class file_filter;

class file{
    
    /**
     * UTF-8 path
     */
    char _path[128 + 1];

public:
    /**
     * 
     */
    file();
    
    /**
     * 
     */
    file(const char* path, int path_length = -1);
    
    /**
     * 
     */
    file(const file&);
    
    /**
     * 
     */
    file(file&&);
    
    /**
     * 
     */
    file& operator= (const file&);
    
    /**
     * 
     */
    file& operator= (file&&);
    
    /**
     * 
     */
    ~file();
    
    /**
     * Проверяет существование файла или каталога.
     * @return true - если файл или каталог существует, иначе false.
     */
    bool exists() const;

    /**
     * Проверяет, является ли текущий файл файлом.
     * 
     * @return 
     *      true - если файл является файлом, иначе false.
     */
    bool is_file() const;
    
    /**
     * Проверяет, является ли текущий файл директорией.
     * 
     * @return 
     *      true - если файл является директорией, иначе false.
     */
    bool is_dir() const;

    /**
     * Возвращает длину файла. (В байтах).
     * 
     * @remark 
     *      Для директорий всегда возвращается 0.
     * 
     * @return 
     *      Длина файла.
     * 
     * @throw sequrity_exception 
     *      Eсли доступ к файлу запрещён.
     */
    uint64_t length() const;

    /**
     * Возвращает значение последнего изменения файла (В миллисекундах).
     * 
     * @return 
     *      Значение последнего изменения в миллисекундах.
     * 
     * @throw sequrity_exception 
     *      Eсли доступ к файлу запрещён.
     */
    uint64_t last_modified() const;
    
    /**
     * Устанавливает значение последнего изменения файла (В миллисекундах).
     * 
     * @return 
     *      true - если операция произошла успешно, иначе false.
     * 
     * @throw sequrity_exception 
     *      Eсли доступ к файлу запрещён.
     */
    bool set_last_modified(uint64_t ms);

    /**
     * Является путь к этому файлу абсолютным.
     * @remark
     *      На Windows абсолютный путь всегда начинается с метки тома "A-Z:\\", например "C:\\"
     *      На Unix всегда с косой черты "/"
     * 
     * @return является ли путь абсолютным.
     */
    bool is_absolute_path() const;

    /**
     * Создаёт новый файл, описанный текущем путём.
     * 
     * @return 
     *      true - если файл создался, false - если файл уже существует.
     * 
     * @throw sequrity_exception 
     *      Eсли доступ к файлу запрещён.
     * 
     * @throw 
     *      io_exception - если произошла ошибка при создании файла.
     */
    bool create_new_file();

    /**
     * Создаёт директорию описанную текущим файлом.
     * 
     * @return 
     *      true - если директория создана, иначе false.
     * 
     * @throw sequrity_exception 
     *      Eсли доступ к файлу запрещён.
     */
    bool mkdir();

    /**
     * Создаёт директорию, и родительские поддиректории, описанные текущим файлом.
     * 
     * @return 
     *      true - если директория создана вместе со всеми поддиректориями, иначе false.
     * 
     * @throw sequrity_exception 
     *      Eсли доступ к файлу запрещён.
     */
    bool mkdirs();

    /**
     * Удаляет файл или директорию описанную текущим файлом.
     * 
     * @return 
     *      true - если файл или директория успешно удалены, иначе false.
     * 
     * @throw sequrity_exception 
     *      Eсли доступ к файлу запрещён.
     */
    bool remove();

    /**
     * Переименовывает файл описанным текущим файлом.
     * 
     * @param new_name
     *      UTF-8 строка с новым именем.
     * 
     * @param new_name_length
     *      необязательный параметр, обозначающий длину строки.
     *      -1 означает, что длина будет вычислена автоматически.
     * 
     * @return 
     *      true - если файл успешно был переименован, иначе false.
     * 
     * @throw sequrity_exception 
     *      Eсли доступ к файлу запрещён.
     */
    bool rename_to(const char* new_name, int new_name_length = -1);

    /**
     * Может ли файл выполняться.
     * 
     * @return 
     *      true - если, файл может быть выполнен, иначе false.
     * 
     * @throw sequrity_exception 
     *      Eсли доступ к файлу запрещён.
     */
    bool can_execute() const;

    /**
     * Может ли файл читать.
     * 
     * @return true 
     *      Eсли, файл может быть прочтён, иначе false.
     * 
     * @throw sequrity_exception 
     *      Eсли доступ к файлу запрещён.
     */
    bool can_read() const;

    /**
     * Можно ли в файл записывать.
     * 
     * @return 
     *      true - если, в файл может быть выполнено запись, иначе false.
     * 
     * @throw sequrity_exception 
     *      Eсли доступ к файлу запрещён.
     */
    bool can_write() const;

    /**
     * Включает=отключает разрешение на выполнение файла, описанного этим файлом.
     * 
     * @param on_off 
     *      Разрешить или запретить выполнение файла.
     * 
     * @return 
     *      true - если операция произошла успешно, иначе false.
     * 
     * @throw sequrity_exception 
     *      Если доступ к файлу запрещён.
     */
    bool set_executable(bool on_off);
    
    /**
     * Включает=отключает разрешение на запись в файл, описанного этим файлом.
     * 
     * @param on_off 
     *      Разрешить или запретить запись в файл.
     * 
     * @return 
     *      true - если операция произошла успешно, иначе false.
     * 
     * @throw sequrity_exception 
     *      Если доступ к файлу запрещён.
     */
    bool set_writable(bool on_off);
    
    /**
     * Включает=отключает разрешение на чтение файла, описанного этим файлом.
     * 
     * @param on_off 
     *      Разрешить или запретить чтение файла.
     * 
     * @return 
     *      true - если операция произошла успешно, иначе false.
     * 
     * @throw 
     *      sequrity_exception - если доступ к файлу запрещён.
     */
    bool set_readable(bool on_off);

    /**
     * @return 
     *      Указатель на строку, содержащее значение пути.
     */
    const char* str_path() const;

    /**
     * Копирует имя файла в передаваемый буфер. 
     * А так же заканчивает строку нуль-терминатором.
     * 
     * @param buf
     *      Буфер в который будет скопировано название файла.
     * 
     * @param bufsize
     *      Размер передаваемого буфера.
     * 
     * @return
     *      Сколько было скопировано символов. (Не включает нулевой).
     */
    int get_name(char buf[], int bufsize) const;
    
    /**
     * Возвращает массив объектов файлов которые находятся в директории, описываемом этим файлом.
     * 
     * @param filter
     *      Фильтр для файлов.
     * 
     * @param allocator
     *      Распределитель памяти, который используется для выделения памяти под результирующий массив.
     * 
     * @return
     *      Массив файлов, который находятся в этой директории описываемым этим файлом, и файлы, которые прошли фильтр.
     */
    array<file> list_files(file_filter* filter = nullptr, tca::allocator* allocator = tca::get_scoped_or_default()) const;
    
    /**
     * Возвращает объект файла, описывающий путь до папки с исполняемым файлом.
     * 
     * @return 
     *      Путь до папки с исполняемым файлом.
     */
    static file runtime_path();

    /**
     * Создаёт объект файла, описывающий форматирубщей строкой.
     * 
     * @return 
     *      Объект файла.
     */
    static file make(const char* format, ...);
    
    /**
     * Создаёт абсолюный путь к файлу.
     * Например:
     * 
     * @code {
     *      file f = file::make_absolute("%s", "./text.txt");
     *      вернёт, как [путь до исполняемого файла программы]/./text.txt
     * }
     * 
     * @return объект файла описывающий абсолютный путь к файлу.
     * 
     * @throw sequrity_exception - если доступ к файлу запрещён.
     */
    static file make_absolute(const char* format, ...);

    /**
     * Возвращает объект, описывающий родительский каталог.
     */
    file get_parent() const;

    /**
     * Возвращает хеш-код для этого файла.
     */
    int64_t hashcode() const;
    
    /**
     * Проверяет, эквивалентен ли передаваймый файл этому файлу.
     * 
     * @return
     *      true - если передаваемый файл равен этому файлу, иначе false.
     */
    bool equals(const file& f) const;

    /**
     * Добавляет путь входящего файла к этому файлу и возвращает новый файл.
     * 
     * @param f
     *      Абстрактный файл, который будет добавлен к этому абстрактному файлу.
     * 
     * @return
     *     Абстрактный файл, путь которого состоит из этого файла плюс входящего файла.
     * 
     * @since 1.1
     */
    file plus(const file& f) const;

    /**
     * @see {@ file::plus(const file&)}
     * @since 1.1
     */
    file operator+ (const file& f) const {
        return this->plus(f);
    }
};


};
#endif//_JSTD_CPP_LANG_IO_FILE_H_