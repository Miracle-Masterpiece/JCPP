#ifndef JSTD_CPP_ZIP_INFLATER_H
#define JSTD_CPP_ZIP_INFLATER_H

#include <zlib.h>

namespace jstd {

class inflater {
    z_stream strm;   /* Поток для разжатия, используемый zlib */
    bool finished;   /* Флаг, указывающий, завершено ли разжатие */

    /**
     * Конструктор копирования удалён
     */
    inflater(const inflater&) = delete;
    
    /**
     * Оператор копирования удалён
     */
    inflater& operator- (const inflater&) = delete;
public:
    /**
     * Конструктор класса.
     * Инициализирует поток для разжатия.
     */
    inflater();

    /**
     * Конструктор перемещения.
     * 
     * @param i
     *      Объект, из которого осуществляется перемещение.
     */
    inflater(inflater&& i);

    /**
     * Оператор присваивания перемещением.
     * 
     * @param i
     *      Объект, из которого происходит перемещение.
     * 
     * @return
     *      Ссылка на текущий объект.
     */
    inflater& operator= (inflater&& i);

    /**
     * Деструктор класса.
     * Завершает поток разжатия и освобождает ресурсы.
     */
    ~inflater();

    /**
     * Устанавливает входные данные для разжатия.
     * 
     * @param data
     *      Указатель на данные для разжатия.
     * 
     * @param size
     *      Размер данных в байтах.
     */
    void set_input(const void* data, size_t size);

    /**
     * Выполняет разжатие данных в выходной буфер.
     * 
     * Возвращает количество данных, записанных в выходной буфер, и обновляет состояние разжатия.
     * 
     * @param outBuffer
     *      Указатель на выходной буфер для разжатых данных.
     * 
     * @param outBufferSize
     *      Размер выходного буфера.
     * 
     * @param finish
     *      Флаг завершения разжатия. Если установлен в true, будет выполнено завершение разжатия.
     * 
     * @return
     *      Количество байт, записанных в выходной буфер.
     * 
     * @throws illegal_state_exception
     *      Если произошла ошибка разжатия.
     */
    size_t inflate(void* outBuffer, size_t outBufferSize, bool finish = false);

    /**
     * Проверяет, нужны ли дополнительные входные данные для разжатия.
     * 
     * @return true, если данные для разжатия закончились и требуется новый ввод, иначе false.
     */
    bool needs_input() const;

    /**
     * Проверяет, завершено ли разжатие данных.
     * 
     * @return true, если разжатие завершено, иначе false.
     */
    bool is_finished() const;

    /**
     * Сбрасывает состояние разжатия, позволяя начать процесс заново.
     */
    void reset();

    /**
     * Устанавливает словарь для разжатия.
     * 
     * Используется, если разжатые данные требуют предварительно установленного словаря.
     * 
     * @param dict
     *      Указатель на данные словаря.
     * 
     * @param dictLength
     *      Длина словаря в байтах.
     */
    void set_dictionary(const void* dict, size_t dictLength);
};

}

#endif//JSTD_CPP_ZIP_INFLATER_H