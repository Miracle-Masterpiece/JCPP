#ifndef JSTD_CPP_LANG_UTILS_IMAGES_IMAGE_H
#define JSTD_CPP_LANG_UTILS_IMAGES_IMAGE_H

#include <cstdint>
#include <allocators/allocator.hpp>

namespace jstd {

/**
 * Класс image представляет собой двумерное изображение в формате массива байт (пикселей),
 * с указанием ширины, высоты и количества цветовых каналов.
 * Поддерживает клонирование, ресайз и управление памятью через пользовательский аллокатор.
 */
class image {
public:
    using byte = unsigned char;
    struct rgba {
        byte r, g, b, a;
        rgba(byte r = 0, byte g = 0, byte b = 0, byte a = 0) : r(r), g(g), b(b), a(a) {}
        rgba(const rgba&)               = default;
        rgba(rgba&&)                    = default;
        rgba& operator=(const rgba&)    = default;
        rgba& operator=(rgba&&)         = default;
        ~rgba()                         = default;
        int to_string(char buf[], int bufsize) const;
    };
    struct rgb {
        byte r, g, b;
        rgb(byte r = 0, byte g = 0, byte b = 0) : r(r), g(g), b(b) {}
        rgb(const rgb&)             = default;
        rgb(rgb&&)                  = default;
        rgb& operator=(const rgb&)  = default;
        rgb& operator=(rgb&&)       = default;
        ~rgb()                      = default;
        int to_string(char buf[], int bufsize) const;
    };
    struct gray {
        byte brightness;
        gray(byte brightness = 0) : brightness(brightness) {}
        gray(const gray&)               = default;
        gray(gray&&)                    = default;
        gray& operator=(const gray&)    = default;
        gray& operator=(gray&&)         = default;
        ~gray()                         = default;
        int to_string(char buf[], int bufsize) const;
    };
private:
    /**
     * Аллокатор, используемый для управления памятью.
     */
    tca::base_allocator* m_allocator;

    union {
        byte*   m_pixels;                    // Указатель на массив пикселей изображения.
        rgba*   m_rgba;
        rgb*    m_rgb;
        gray*   m_gray;
    };
    
    /**
     * Ширина изображения в пикселях.
     */
    int32_t m_width;
    
    /**
     * Высота изображения в пикселях.
     */
    int32_t m_height;
    
    /**
     * Количество цветовых каналов (например, 3 = RGB, 4 = RGBA).
     */
    int8_t  m_channels;

    /**
     * Освобождает ресурсы, связанные с изображением.
     */
    void cleanup();

    /**
     * Для создания view на массив байтов.
     */
    image(image::byte* data, int32_t w, int32_t h, int8_t channels);
    
    /**
     * Для захвата владения над массивом байтов.
     */
    image(image::byte* data, tca::base_allocator* allocator, int32_t w, int32_t h, int8_t channels);

public:
    /**
     * Конструктор по умолчанию.
     * Создаёт пустое изображение без выделения памяти.
     */
    image();

    /**
     * Создаёт изображение заданного размера и количества каналов с использованием указанного аллокатора.
     *
     * @param allocator 
     *      Аллокатор для выделения памяти.
     * 
     * @param width 
     *      Ширина изображения.
     * 
     * @param height 
     *      Высота изображения.
     * 
     * @param channels 
     *      Количество каналов (например, 3 = RGB, 4 = RGBA).
     */
    image(int32_t width, int32_t height, int8_t channels, tca::base_allocator* allocator = tca::get_scoped_or_default());

    /**
     * Конструктор копирования.
     *
     * @param other 
     *      Другое изображение для копирования.
     */
    image(const image& other);

    /**
     * Конструктор перемещения.
     *
     * @param other 
     *      Другое изображение, ресурсы которого будут переняты.
     */
    image(image&& other);

    /**
     * Оператор копирующего присваивания.
     *
     * @param other 
     *      Другое изображение.
     * 
     * @return 
     *  Ссылка на это изображение.
     */
    image& operator=(const image& other);

    /**
     * Оператор перемещающего присваивания.
     *
     * @param other 
     *      Другое изображение.
     * 
     * @return 
     *      Ссылка на это изображение.
     */
    image& operator=(image&& other);

    /**
     * Деструктор. 
     * Освобождает выделенную память.
     */
    ~image();

    /**
     * Возвращает указатель на пиксели изображения.
     *
     * @return 
     *      Указатель на первый байт массива пикселей.
     */
    byte* pixels();

    /**
     * Возвращает указатель на пиксели изображения (константная версия).
     *
     * @return 
     *      Указатель на первый байт массива пикселей (только для чтения).
     */
    const byte* pixels() const;

    /**
     * Возвращает ширину изображения.
     *
     * @return 
     *      Ширина в пикселях.
     */
    int32_t get_width() const;

    /**
     * Возвращает высоту изображения.
     *
     * @return 
     *      Высота в пикселях.
     */
    int32_t get_height() const;

    /**
     * Возвращает количество цветовых каналов.
     *
     * @return 
     *      Количество каналов (например, 3 или 4).
     */
    int8_t get_channels() const;

    /**
     * Возвращает новое изображение, масштабированное до заданных размеров.
     *
     * @note
     *      Если передаваемый аллокатор равен nullptr и текущее изображение является view, то функция вернёт пустое изображение.
     * 
     * @param neww 
     *      Новая ширина.
     * 
     * @param newh 
     *      Новая высота.
     * 
     * @param allocator 
     *      Необязательный аллокатор; если не указан, используется текущий.
     * 
     * @return 
     *      Новое масштабированное изображение.
     */
    image resize(int32_t neww, int32_t newh, tca::base_allocator* allocator = nullptr) const;

    /**
     * 
     */
    rgb& get_rgb(int32_t x, int32_t y);

    /**
     * 
     */
    const rgb& get_rgb(int32_t x, int32_t y) const;

    /**
     * 
     */
    rgba& get_rgba(int32_t x, int32_t y);

    /**
     * 
     */
    const rgba& get_rgba(int32_t x, int32_t y) const;

    /**
     * 
     */
    gray& get_gray(int32_t x, int32_t y);
    
    /**
     * 
     */
    const gray& get_gray(int32_t x, int32_t y) const;

    /**
     * Записывает краткое строковое представление изображения в указанный буфер.
     * Формат строки может включать размер и количество каналов.
     *
     * @param buf 
     *      Буфер, в который будет записана строка.
     * 
     * @param bufsize 
     *      Размер буфера.
     * 
     * @return 
     *      Количество записанных символов.
     */
    int to_string(char buf[], int bufsize) const;

    /**
     * Создаёт копию изображения.
     * 
     * @note
     *      Если передаваемый аллокатор равен nullptr и текущее изображение является view, то функция вернёт пустое изображение.
     * 
     * @param allocator 
     *      Необязательный аллокатор. 
     *      Eсли не указан, используется текущий.
     * 
     * @return 
     *      Новое изображение, содержащее копию пикселей.
     */
    image clone(tca::base_allocator* allocator = nullptr) const;

    /**
     * Создаёт view изображения на передаваемую память.
     * 
     * @note
     *      Данная функция создаёт только view на массив, при этом объект не становится владельцем памяти!
     * 
     * @param data
     *      Указатель на массив байтов изображения.
     * 
     * @param width
     *      Ширина изображения.
     * 
     * @param height
     *      Высота изображения.
     * 
     * 
     * @param channles
     *      Количество каналов изображения.
     * 
     * @throw null_pointer_exception
     *      Если data равна nullptr.
     */
    static image make_view(image::byte* data, int32_t width, int32_t height, int8_t channels);

    /**
     * Захватывает указатель на массив изображения в своё владение. 
     * 
     * @note
     *      После вызова этой функции, изображение становится владельцем указателя.
     *      Указатель будет освобождён в момент вызова декструктора изображения.
     * 
     * @param data
     *      Указатель на массив байтов изображения.
     * 
     * @param allocator
     *      Указатель на аллокатор, которым был выделен блок памяти {@param data}
     * 
     * @param width
     *      Ширина изображения.
     * 
     * @param height
     *      Высота изображения.
     * 
     * @param channles
     *      Количество каналов изображения.
     * 
     * @throw null_pointer_exception
     *      Если data равна nullptr.
     *      Если allocator равен nullptr.
     */
    static image lock(image::byte* data, tca::base_allocator* allocator, int32_t width, int32_t height, int8_t channels);
};
    

}

#endif//JSTD_CPP_LANG_UTILS_IMAGES_IMAGE_H