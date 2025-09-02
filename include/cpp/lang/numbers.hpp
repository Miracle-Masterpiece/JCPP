#ifndef JSTD_CPP_NUMBERS_H
#define JSTD_CPP_NUMBERS_H

#include <cstdint>
#include <climits>
#include <cpp/lang/math/math.hpp>
#include <cpp/lang/utils/traits.hpp>

namespace jstd 
{

namespace num 
{

    /**
     * Преобразует значение float в его побитовое представление как int32_t.
     *
     * Эта функция копирует байты из значения float в int32_t без изменения битов.
     * Используется, когда необходимо проанализировать или сохранить точное битовое
     * представление числа с плавающей точкой.
     *
     * @param v 
     *      Значение типа float, которое нужно интерпретировать как int32_t.
     * 
     * @return 
     *      Побитовое представление float в виде int32_t.
     * 
     * @since 1.0
     */
    int32_t float_to_int_bits(float v);

    /**
     * Преобразует значение double в его побитовое представление как int64_t.
     *
     * Эта функция копирует байты из значения double в int64_t без изменения битов.
     * Используется, когда необходимо сохранить или передать точное битовое
     * представление double.
     *
     * @param v 
     *      Значение типа double, которое нужно интерпретировать как int64_t.
     * 
     * @return 
     *      Побитовое представление double в виде int64_t.
     * 
     * @since 1.0
     */
    int64_t double_to_long_bits(double v);

    /**
     * Преобразует строковое представление числа в int.
     * 
     * @param str
     *      Строка, представляющая число.
     * 
     * @param radix
     *      Система счисления в которой записано строковое число.
     *      2  - двоичная.
     *      8  - восьмиричная.
     *      10 - десятиричная.
     *      16 - шеснадцатеричная.
     * 
     * @return
     *      Преобразованное число из строки.
     * 
     * @throws numper_format_exception
     *      Если строка не может быть преобразована в тип int.
     * 
     * @since 1.1
     */
    int64_t parse_int(const char* str, int radix = 10);

    /**
     * Преобразует строковое представление числа с правающей точкой в float.
     * 
     * @param str
     *      Строка, представляющая число с плавающей точкой.
     * 
     * @return
     *      Преобразованное число с плавающей точкой из строки.
     * 
     * @throws numper_format_exception
     *      Если строка не может быть преобразована в тип float.
     * 
     * @since 1.1
     */
    float parse_float(const char* value);
    
    /**
     * Преобразует строковое представление числа двойной точности в double.
     * 
     * @param str
     *      Строка, представляющая число с двойной точностью.
     * 
     * @return
     *      Преобразованное число двойной точности из строки.
     * 
     * @throws numper_format_exception
     *      Если строка не может быть преобразована в тип double.
     * 
     * @since 1.1
     */
    double parse_double(const char* value);

    /**
     * Проверяет, является ли {@param c} цифрой из шеснадцатеричной системы счисления.
     * 
     * @return
     *      true - если символ является цифрой в шеснадцатеричной системе счисления; иначе - false.
     * 
     * @since 1.1
     */
    bool is_hex_digit(char c);

    /**
     * Проверяет, является ли {@param c} цифрой из десятиричной системы счисления.
     * 
     * @return
     *      true - если символ является цифрой в десятиричной системе счисления; иначе - false.
     * 
     * @since 1.1
     */
    bool is_digit(char c);

    /**
     * @since 2.0
     */
    template<typename T, typename = typename enable_if<is_primitive<T>::value>::type>
    int32_t to_binary_string(char buf[], int32_t bufsize, const T& v) {
        const std::size_t bits = sizeof(T) * CHAR_BIT;
        bool has_add = false;
        int32_t offset = 0;
        for (int32_t i = bits - 1; i >= 0; --i) {
            unsigned char bit = ((v >> i) & 0x1);
            if (bit)
                has_add = true;
            if (has_add) {
                if (offset < bufsize)    
                    buf[offset++] = '0' + bit;
                else
                    break;
            }
        }
        buf[math::min(offset, bufsize - 1)] = '\0';
        return 0;
    }
   
}//namespace num

}//namespace jstd

#endif//JSTD_CPP_NUMBERS_H