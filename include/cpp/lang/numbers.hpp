#ifndef JSTD_CPP_NUMBERS_H
#define JSTD_CPP_NUMBERS_H

#include <cstdint>

namespace jstd {

namespace num {

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
     */
    int64_t double_to_long_bits(double v);

}//namespace num

}//namespace jstd

#endif//JSTD_CPP_NUMBERS_H