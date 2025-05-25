#ifndef _JSTD_CPP_LANG_IO_FILESYSTEM_H_
#define _JSTD_CPP_LANG_IO_FILESYSTEM_H_

#include <cstdint>
#include <cstdio>
#include <allocators/base_allocator.hpp>
#include <cpp/lang/io/utility.hpp>
#include <cpp/lang/io/constants.hpp>

#if defined(__linux__) || defined(__APPLE__)
    #include <dirent.h>
#elif defined(_WIN32)
    #include <windows.h>
#endif

namespace jstd {

class filesystem {
public:
#if defined(__linux__) || defined(__APPLE__) 
    static const char FILE_SEPARATOR = '/';
#elif defined(_WIN32)
    static const char FILE_SEPARATOR = '\\';
#endif

    static const int MAX_LENGTH_PATH = io::constants::MAX_LENGTH_PATH;
    
    static bool     exists(const char* path, int path_length = -1);
    static bool     is_file(const char* path, int path_length = -1);
    static bool     is_dir(const char* path,  int path_length = -1);
    static int      runtime_path(char buf[], int bufsize);
    static int      normalize_path(char path[], int path_length = -1);
    static bool     is_separator(char ch);
    static bool     is_absolute(const char* path, int path_length = -1);
    static uint64_t length(const char* path, int path_length = -1);
    static uint64_t last_modified(const char* path, int path_length = -1);
    static bool     set_last_modified(const char* path, uint64_t ms_time, int path_length = -1);
    static bool     create_new_file(const char* path, int path_length = -1);
    static bool     mkdir(const char* path, int path_length = -1);
    static bool     remove(const char* path, int path_length = -1);
    static bool     mkdirs(const char* path, int path_length = -1);

    static bool     can_execute(const char* path,    int path_length = -1);
    static bool     can_read(const char* path,       int path_length = -1);    
    static bool     can_write(const char* path,      int path_length = -1);

    static bool     set_executable(const char* path, bool executable,    int path_length = -1);
    static bool     set_readable(const char* path,   bool readable,      int path_length = -1);
    static bool     set_writable(const char* path,   bool writable,      int path_length = -1);

    static bool     rename_to(const char* old_path, const char* new_path, int old_path_length = -1, int new_path_length = -1);

    static int      count_files_in_directory(const char* path, file_filter* filter, int path_length = -1);

    static FILE*    open(const char* path, const char* mark, int path_length = -1, int mark_length = -1);
    static void     close(FILE*);
    static void     flush(FILE*);

    static char const* const* list_files(
        const char* path, 
        tca::base_allocator* allocator, 
        file_filter* filter = nullptr, 
        int path_length = -1);

    static void free_list_files(char const* const* files, tca::base_allocator* allocator);

    static int get_parent(const char* path, char out_path[], int buf_size, int path_length = -1);
};

// directory_entry — класс который хранит имя файла или папки.
class directory_entry {
    char _name[filesystem::MAX_LENGTH_PATH]; // UTF-8 строка, имя файла/папки.
public:
    // Создаёт directory_entry с указанным путём.
    // Если path_length -1, то считаем, что строка заканчивается на '\0'.
    directory_entry(const char* path, int path_length = -1);

    // Копирует другой directory_entry. Просто копирует строку.
    directory_entry(const directory_entry& other);

    // Перемещает другой directory_entry. Но на самом деле просто копирует строку.
    directory_entry(directory_entry&& other);

    // Копирует данные из другого directory_entry.
    directory_entry& operator=(const directory_entry& other);

    // Перемещает другой directory_entry. Но по факту снова копирует строку.
    directory_entry& operator=(directory_entry&& other);

    // Уничтожает directory_entry.
    ~directory_entry();

    // Возвращает имя файла или папки (UTF-8).
    const char* get_name() const {
        return _name;
    }
};

// directory_iterator — класс, который перебирает файлы в папке.
class directory_iterator {
#if defined(__linux__) || defined(__APPLE__)
    DIR* _dir;                  // Открытая папка.
    struct dirent* _entry;      // Текущий файл/папка.
#elif defined(_WIN32)
    HANDLE _dir;                // Дескриптор поиска файлов.
    WIN32_FIND_DATAW _entry;    // Инфа о текущем файле.
    bool _end;                  // true, если файлы закончились.
#endif

    // Закрывает директорию, если открыта.
    void close();

    // Копировать нельзя.
    directory_iterator(const directory_iterator&) = delete;
    directory_iterator& operator=(const directory_iterator&) = delete;

public:
    // Открывает папку и начинает её перебирать.
    // Если path не указан, то создаётся итератор, как бы указывающий на конец.
    directory_iterator(const char* path = nullptr, int path_length = -1);

    // Перемещает другой directory_iterator.
    directory_iterator(directory_iterator&& other);

    // Перемещающее присваивание.
    directory_iterator& operator=(directory_iterator& other);

    // Закрывает директорию, если открыта.
    ~directory_iterator();

    // Проверяет, не один ли это итератор.
    bool operator!=(const directory_iterator& it) const;

    // Переход к следующему файлу.
    directory_iterator& operator++();

    // Возвращает текущий файл/папку.
    directory_entry operator*() const;
};

}
#endif//_JSTD_CPP_LANG_IO_FILESYSTEM_H_
