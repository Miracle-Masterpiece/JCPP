#ifndef JSD_CPP_LANG_IO_FILE_MAPPED_BYTE_BUFFER_H
#define JSD_CPP_LANG_IO_FILE_MAPPED_BYTE_BUFFER_H

#include <cpp/lang/io/bytebuffer.hpp>
#include <internal/io/file_descriptor.hpp>
#include <cpp/lang/utils/cond_compile.hpp>
#include <cpp/lang/io/file.hpp>
#include <cpp/lang/io/channel.hpp>

namespace jstd
{

/**
 * Защита страниц отображённой памяти.
 */
enum struct fmap_mode {
   READ_ONLY,   /*Только чтение*/
   READ_WRITE   /*Чтение и запись*/
};

/**
 * Тип открытия файла.
 */
namespace open_option 
{
   const int CREATE         = 1 << 0;       /*Создаёт новый или открывает существующий файл*/
   const int APPEND         = 1 << 1;       /*Добавляет в уже существующий файл*/
   const int READ           = 1 << 2;       /*Открывает существующий файл на чтение*/
   const int WRITE          = 1 << 3;       /*Открывает существующий файл на запись*/
   const int READ_WRITE     = READ | WRITE; /*Открывает существующий файл на чтение и запись*/
};

class file_channel;
/**
 * @version 1.1
 */
class mapped_byte_buffer : public byte_buffer {
    
    /**
     * 
     */
    friend file_channel;
    
    /**
     * 
     */
    JSTD_WIN_CODE(
        typedef void* win_handle_t;
        win_handle_t m_mapped_hanle;
    )
    
    /**
     * @internal
     *      Internal Only!
     * Базовый указатель на начало страницы.
     */
    void* m_base;

    /**
     * 
     */
    mapped_byte_buffer(const mapped_byte_buffer&)               = delete;
    
    /**
     * 
     */
    mapped_byte_buffer& operator= (const mapped_byte_buffer&)   = delete;
    
    /**
     * @internal
     * 
     * @param page_base
     *      Указатель на выровненный адрес страницы. (Если платформе нужен именно выровненный).
     *      Этот указатель должен быть указателем, который возвращает ОС в качестве указателя на отображемую память.
     *      Например, на Linux - это mmap, а на Windows - MapViewOfFile.
     * 
     * @param base
     *      Смещённый указатель на память.
     *  
     * @param capacity
     *      Размер отображаемого блока памяти.
     * 
     * @param mode
     *      Режим открытия.
     */
    mapped_byte_buffer(void* page_base, void* base, int64_t capacity, fmap_mode mode);
    
    /**
     * @internal
     * Отменяет отображение на память.
     */
    void cleanup();

public:
    /**
     * Создаёт пустой сопоставляемый байтовый буфер,
     * не привязывая к конкретному файлу.
     */
    mapped_byte_buffer();    
    
    /**
     * Перемещает данные из другого сопоставляемого байтового буфера в этот.
     * При этом другой объект становится невалидным.
     */
    mapped_byte_buffer(mapped_byte_buffer&&);
    
    /**
     * Отменяет текущее сопоставление файла на память 
     * и перемещает данные из другого байтового буфера в этот.
     * 
     * При этом другой объект становится невалидным.
     * 
     * @return
     *      Этот объект.
     */
    mapped_byte_buffer& operator= (mapped_byte_buffer&&);
    
    /**
     * Отменяет сопоставление файла на память.
     */
    ~mapped_byte_buffer();

    /**
     * Принудительно записывает все изменения, внесенные в содержимое этого буфера, 
     * на запоминающее устройство, содержащее сопоставленный файл.
     * 
     * @return 
     *      Ссылка на этот объект
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     */
    mapped_byte_buffer& force();
};

/**
 * Файловый канал, который позволяет создавать, изменять размер и отображать файл в память.
 * 
 * @note
 *      Для освобождения ресурсов внутри этого класса, необходимо вызывать функцию {@code file_channel::close()}
 *      Деструктор не освобождает ресурсы!
 * 
 * @version 1.1
 */
class file_channel : public channel {
    /**
     * 
     */
    static const int ZERO_FLAG = 0;

    /**
     * Дескриптор файла.
     */
    internal::file_descriptor m_fd;

    /**
     * Флаги открытия.
     */
    int m_oflags;

    /**
     * 
     */
    file_channel(const file_channel&)             = delete;
    
    /**
     * 
     */
    file_channel& operator= (const file_channel&) = delete;
    
    /**
     * @internal
     * 
     * Проверяет, не равен ли m_fd нулевому дескриптору.
     * И если равен, бросает исключение.
     */
    void require_fd() const;

public:    

    /**
     * Создаёт пустой объект отображённого файла, который не привязан ни к одному файлу.
     */
    file_channel();
    
    /**
     * Создаёт объект и открывает файл.
     * 
     * @see mmaped_file::open
     */
    file_channel(const jstd::file& path, int option);
    
    /**
     * Перемещает открытый дескриптор из другого объекта в этот.
     */
    file_channel(file_channel&&);
    
    /**
     * Перемещает открытый дескриптор из другого объекта в этот.
     * При этом этот дескриптор закрывается.
     */
    file_channel& operator= (file_channel&&);
    
    /**
     * @note
     *      Не закрывает этот файловый канал.
     *      Для закрытия канала, необходимо явно вызвать функцию {@code file_channel::close()}
     */
    ~file_channel();
    
    /**
     * Открывает файл.
     * 
     * @param path
     *      Путь к файлу.
     * 
     * @param option
     *      Опции открытия.
     *      @see jstd::open_option
     * 
     * @throws file_not_found_exception
     *      Если опция указывает на добавление в файл и файла не существует.
     * 
     * @throws sequrity_exception
     *      Если доступ к файлу запрещён.
     * 
     * @throws io_exception
     *      Если попытка открыть файл дважды. 
     *      Если произошла ошибка ввода/вывода.
     */
    void open(const jstd::file& path, int option);
    
    /**
     * Возвращает длину файла. (В байтах).
     * 
     * @throws io_exception
     *      Если файл не был открыт.
     *      Если произошла ошибка ввода/вывода.
     */
    int64_t size() const;

    /**
     * Устанавливает новую длину файла.
     * 
     * @param length
     *      Новая длина файла.
     * 
     * @throws sequrity_exception
     *      Если доступ к файлу запрещён.
     * 
     * @throws io_exception
     *      Если файл не был открыт.
     *      Если произошла ошибка ввода/вывода.
     */
    file_channel& truncate(int64_t length);
    
    /**
     * Отображает файл в память и возвращает ввиде mapped_byte_buffer.
     * 
     * @param mode
     *      Режим отображения.
     * 
     * @param offset
     *      Смещение от начала файла.
     * 
     * @param length
     *      Размер отображённой части.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     */
    mapped_byte_buffer map(fmap_mode mode, int64_t offset, int64_t length);
    
    /**
     * Проверяет, является ли этот дескриптор, дескриптором открытого файла.
     * 
     * @return
     *      true - если файл открыт, иначе - false.
     */
    bool is_open() const override;

    /**
     * Закрывает этот файловый канал.
     * 
     * @throws io_exception
     *      Если произошда ошибка ввода/вывода.
     */
    void close() override;
};

}

#endif//JSD_CPP_LANG_IO_FILE_MAPPED_BYTE_BUFFER_H