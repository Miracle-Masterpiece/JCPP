#ifndef _JSTD_CPP_LANG_UTILS_HASH_H_
#define _JSTD_CPP_LANG_UTILS_HASH_H_

#include <cpp/lang/utils/traits.hpp>
#include <cstdint>

namespace jstd {

/**
 * @brief Хэш-функция по умолчанию для типа K.
 * 
 * @tparam K 
 *      Тип ключа.
 * 
 * @note 
 *      В текущей реализации всегда возвращает значение адреса.
 */
template<typename K>
struct hash_for {
    /**
     * @brief Вычисляет хэш-код для переданного ключа.
     * 
     * @param key 
     *      Ключ, для которого вычисляется хэш.
     * 
     * @return 
     *      uint64_t Хэш-код.
     */
    uint64_t operator() (const K& key) const {
        if (is_primitive<K>::value)
            return (uint64_t) key;
        return (uint64_t) &key;
    }
};

/**
 * @brief Компаратор равенства по умолчанию.
 * 
 * @tparam K 
 *      Тип значения, для которого выполняется сравнение.
 */
template<typename K>
struct equal_to {
    /**
     * @brief Проверяет равенство двух значений.
     * 
     * @param v1 
     *      Первое значение.
     * 
     * @param v2 
     *      Второе значение.
     * 
     * @return 
     *      true, если значения равны, иначе false.
     */
    bool operator() (const K& v1, const K& v2) const {
        return v1 == v2;
    }
};

#undef ____JSTD_MAKE_PRIMITIVE_HASH_FOR_AND_EQUAL_TO___
}
#endif//_JSTD_CPP_LANG_UTILS_HASH_H_