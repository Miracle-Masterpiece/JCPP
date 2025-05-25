#ifndef JSTD_CPP_LANG_UTILS_OBJECTS_H
#define JSTD_CPP_LANG_UTILS_OBJECTS_H

#include <cstdint>
#include <cpp/lang/utils/hash.hpp>

namespace jstd {

class null_pointer_exception;
class illegal_argument_exception;

template<typename T>
void throw_except(const char* format, ...);

template<std::size_t>
class cstr_buf;
    
template<typename T>
cstr_buf<T::TO_STRING_MIN_BUFFER_SIZE> obj_to_cstr_buf(const T& object);

template<std::size_t BUF_SIZE>
class cstr_buf {
    char m_strbuf[BUF_SIZE];
    template<typename T>
    friend cstr_buf<T::TO_STRING_MIN_BUFFER_SIZE> obj_to_cstr_buf(const T& object);
public:
    /**
     * Неявное преобразование к C-style строке const char*.
     *
     * Позволяет использовать cstr_buf в функциях, ожидающих const char*,
     * таких как printf, puts, std::cout <<, и т.п.
     *
     * @return Указатель на внутренний символьный буфер.
     */    
    operator const char*() {
        return m_strbuf;
    }
};
    
    /**
     * Формирует строковое представление объекта во внутренний буфер фиксированного размера.
     *
     * Универсальная функция, вызывающая метод to_string у объекта object,
     * передавая ему буфер и его размер. Метод to_string должен иметь сигнатуру:
     *
     * int to_string(char* buffer, int buffer_size) const;
     *
     * Возвращаемый тип — cstr_buf<STR_BUF_SIZE> — содержит C-style строку,
     * готовую к использованию без дополнительного выделения памяти.
     *
     * @tparam T 
     *      Тип объекта, поддерживающего метод to_string(char*, int) const.
     * 
     * @tparam STR_BUF_SIZE 
     *      Размер буфера, выделяемого под строку. По умолчанию — 64 байта.
     *
     * @param object 
     *      Объект, строковое представление которого требуется получить.
     * 
     * @return 
     *      Объект cstr_buf<STR_BUF_SIZE>, содержащий C-style строку.
     */
    template<typename T>
    cstr_buf<T::TO_STRING_MIN_BUFFER_SIZE> obj_to_cstr_buf(const T& object) {
        cstr_buf<T::TO_STRING_MIN_BUFFER_SIZE> a;
        object.to_string(a.m_strbuf, T::TO_STRING_MIN_BUFFER_SIZE);
        return a;
    }


namespace objects {

    /**
     * @brief Вычисляет хеш-код для массива элементов типа T.
     * 
     * Эта функция принимает указатель на массив и вычисляет его хеш-код.
     * 
     * Если длина массива не указана (len == -1), функция будет
     * вычислять хеш-код, пока не встретит нулевой байт (подразумевается, что массив заканчивается нулём, как C-строка).
     * 
     * @tparam T 
     *      Тип элементов массива.
     * 
     * @param array 
     *      Указатель на массив элементов типа T.
     * 
     * @param len 
     *      Длина массива. Если равна -1, хеш вычисляется до первого нулевого байта.
     * 
     * @return 
     *      uint64_t Хеш-код массива.
     * 
     * @warning 
     *      Если массив не завершается нулём, а длина не указана, может случиться взрыв.
     * 
     * @example
     * 
     * char str[] = "Hello, C++";
     * uint64_t hash = hashcode(str);
     * std::cout << "Hash: " << hash << "\n";
     */
    template<typename T, typename HASH_FOR = hash_for<T>>
    uint64_t hashcode(const T* array, int64_t len = -1) {
#ifndef NDEBUG
        if (array == nullptr)
            throw_except<null_pointer_exception>("array must be != null");        
#endif//NDEBUG
        uint64_t hash = 0xcbf29ce484222325;
        const HASH_FOR hash_calculater;
        if (len == -1) {
            while (*array) 
                hash = (hash ^ hash_calculater(*(array++))) * 0x100000001b3;
        } else {
            for (int64_t i = 0; i < len; ++i)
                hash = (hash ^ hash_calculater(array[i])) * 0x100000001b3;
        }

        return hash;
    }
    
    /**
     * @see 
     *      template<typename T>
     *      uint64_t hashcode(const T* array, int64_t len = -1);
     */
    template<>
    uint64_t hashcode<float>(const float* array, int64_t len);

    /**
     * @see 
     *      template<typename T>
     *      uint64_t hashcode(const T* array, int64_t len = -1);
     */
    template<>
    uint64_t hashcode<double>(const double* array, int64_t len);

    /**
     * @brief Сравнивает два массива элементов типа T на равенство.
     * 
     * Функция принимает два указателя на массивы и их длину,
     * после чего посимвольно сравнивает их содержимое.
     * Если массивы полностью совпадают, возвращает true.
     * Если есть хоть одно несовпадение — false.
     * 
     * @tparam T 
     *      Тип элементов массива.
     * 
     * @param a1 
     *      Указатель на первый массив. Должен быть != nullptr.
     * 
     * @param a2 
     *      Указатель на второй массив. Должен быть != nullptr.
     * 
     * @param len 
     *      Длина массивов, подлежащая сравнению.
     * 
     * @return 
     *      true, если массивы идентичны, иначе false.
     * 
     * @throws null_pointer_exception 
     *      Если один из указателей равен nullptr.
     * 
     * @example
     * char str1[] = "Hello, C++";
     * char str2[] = "Hello, C++";
     * bool result = equals(str1, str2, 10);
     * std::cout << (result ? "equal" : "not equal") << std::endl;
     */
    template<typename T, typename EQUAL_TO = equal_to<T>>
    bool equals(const T* a1, const T* a2, std::size_t len) {
#ifndef NDEBUG
    if (a1 == nullptr) throw_except<null_pointer_exception>("a1 must be != null");
    if (a2 == nullptr) throw_except<null_pointer_exception>("a2 must be != null");
#endif//NDEBUG
        const EQUAL_TO equals_to;
        for (std::size_t i = 0; i < len; ++i)
            if (!equals_to(a1[i], a2[i]))
                return false;
        return true;
    }
}//namespace objects

}//namespace jstd

#endif//JSTD_CPP_LANG_UTILS_OBJECTS_H