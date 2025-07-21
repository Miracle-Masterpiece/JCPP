#ifndef JSTD_CPP_LANG_UTILS_IMAGES_IMAGE_H
#define JSTD_CPP_LANG_UTILS_IMAGES_IMAGE_H

#include <cstdint>
#include <allocators/base_allocator.hpp>

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
        rgb(byte r = 0, byte g = 0, byte b = 0, byte a = 0) : r(r), g(g), b(b) {}
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
    tca::base_allocator* m_allocator;    // Аллокатор, используемый для управления памятью.
    
    union {
        byte*   m_pixels;                    // Указатель на массив пикселей изображения.
        rgba*   m_rgba;
        rgb*    m_rgb;
        gray*   m_gray;
    };
    

    int32_t m_width;                     // Ширина изображения в пикселях.
    int32_t m_height;                    // Высота изображения в пикселях.
    int8_t  m_channels;                  // Количество цветовых каналов (например, 3 = RGB, 4 = RGBA).

    /**
     * Освобождает ресурсы, связанные с изображением.
     */
    void cleanup();

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

    rgb& get_rgb(int32_t x, int32_t y);
    rgba& get_rgba(int32_t x, int32_t y);

    const rgb& get_rgb(int32_t x, int32_t y) const;
    const rgba& get_rgba(int32_t x, int32_t y) const;

    gray& get_gray(int32_t x, int32_t y);
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
     * @param allocator 
     *      Необязательный аллокатор. 
     *      Eсли не указан, используется текущий.
     * 
     * @return 
     *      Новое изображение, содержащее копию пикселей.
     */
    image clone(tca::base_allocator* allocator = nullptr) const;
};
    

}

#endif//JSTD_CPP_LANG_UTILS_IMAGES_IMAGE_H