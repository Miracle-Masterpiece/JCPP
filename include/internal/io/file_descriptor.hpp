/*Занятно, похоже это самый чистый внутренний класс*/

#ifndef JSTD_INTERNAL_IO_FILE_DESCRIPTOR_H
#define JSTD_INTERNAL_IO_FILE_DESCRIPTOR_H

namespace jstd
{

namespace internal 
{

/**
 * @internal
 * Представляет собой класс обёртку файлового дескриптора.
 * Класс предоставляет безопасное открытые файла и закрытые в деструкторе.
 */
class file_descriptor {
    file_descriptor(const file_descriptor&)             = delete;
    file_descriptor& operator=(const file_descriptor&)  = delete;
    
    /**
     * Внутренний дескриптор.
     */
    int fd;
    
    /**
     * @internal
     */
    file_descriptor(int fd);
    
    /**
     * @internal
     */
    void cleanup();
public:
    /**
     * Нулевой дескриптор.
     */
    static const file_descriptor NULL_FD;
    
    /**
     * Создаёт дескриптор не привязанный к файлу.
     */
    file_descriptor();
    
    /**
     * Открывает файл привязывает к текущему объекту.
     * 
     * @param u8papth
     *      Путь к файлу в формате UTF-8.
     * 
     * @param oflags
     *      Флаги открытия.
     * 
     * @param pmode
     *      Файлы защиты файла. (Дают эффект только про создании нового файла).
     */
    file_descriptor(const char* u8path, int oflags, int pmode);
    
    /**
     * Перемещет дескриптор из другого объекта.
     */
    file_descriptor(file_descriptor&&);
    
    /**
     * Перемещет дескриптор из другого объекта.
     * При этом закрывая текущий.
     */
    file_descriptor& operator=(file_descriptor&&);
    
    /**
     * Закрывает открытый файловый дескриптор.
     */
    ~file_descriptor();
    
    /**
     * Возвращает числовой идентификатор дескриптора.
     */
    int get_fd() const;
    
    /**
     * Проверяет, равен ли этот дескриптор другому.
     * 
     * @return 
     *      true, если внутренний идентификатор совпадает, иначе false.
     */
    bool operator==(const file_descriptor&) const;
    
    /**
     * Проверяет, не равен ли этот дескриптор другому.
     * 
    * @return 
     *      true, если внутренний идентификатор не совпадает, иначе false.
     */
    bool operator!=(const file_descriptor&) const;
};

}

}

#endif//JSTD_INTERNAL_IO_FILE_DESCRIPTOR_H