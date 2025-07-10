#ifndef _ALLOCATORS_UTILS_H_
#define _ALLOCATORS_UTILS_H_

#include <cstdint>
#include <utility>
#include <cpp/lang/system.hpp>
#include <cpp/lang/utils/comparator.hpp>
#include <cstring>
#include <cpp/lang/utils/traits.hpp>

namespace jstd {

class null_pointer_exception;
class illegal_argument_exception;

template<typename T>
void throw_except(const char* format, ...);

namespace utils {

    /**
     * Меняет порядок байт.
     * 
     * @param x
     *      Значение, у которого поменять порядок байт.
     * 
     * @return
     *      Значение с изменённым порядком байт.
     * 
     * @code 
     * {
     *      int32_t x = 0x11223344;
     *      int swapped_x = bswap(x);
     *      //x == 0x44332211
     * }
     * 
     * @since 1.1
     * 
     * @IntrinsicCandidate
     */
    template<typename T>
    T bswap(T x) {
        unsigned char* v = reinterpret_cast<unsigned char*>(&x);
        const std::size_t T_SIZE = sizeof(x);
        const std::size_t T_HALF_SIZE = T_SIZE / 2;
        unsigned char tmp;
        for (std::size_t i = 0; i < T_HALF_SIZE; ++i) {
            tmp     = v[i];
            v[i]    = v[T_SIZE - i - 1];
            v[T_SIZE - i - 1] = tmp;
        }
        return x;
    }

    /**
     * Оптимизированная версия функции для изменения порядка байт в 16 битной переменной.
     * @see template<typename T> bswap(T x);
     * @since 1.1
     */
    template<>
    inline uint16_t bswap<uint16_t>(uint16_t x) {
        return 
        (((x & 0xff00) >> 8)) |
        (((x & 0x00ff) << 8)); 
    }

    /**
     * Оптимизированная версия функции для изменения порядка байт в 32 битной переменной.
     * @see template<typename T> bswap(T x);
     * @since 1.1
     */
    template<>
    inline uint32_t bswap<uint32_t>(uint32_t x) {
        return 
            bswap<uint16_t>(((x >> 16) & 0xffff)) | 
            bswap<uint16_t>((x & 0xffff)) << 16;
    }

    /**
     * Оптимизированная версия функции для изменения порядка байт в 64 битной переменной.
     * @see template<typename T> bswap(T x);
     * @since 1.1
     */
    template<>
    inline uint64_t bswap<uint64_t>(uint64_t x) {
        return 
        (uint64_t) bswap<uint32_t>((uint32_t) ((x >> 32) & 0xffffffff)) | 
        (uint64_t) bswap<uint32_t>((uint32_t) (x & 0xffffffff)) << 32;
    }

    /**
     * Оптимизированная версия функции для изменения порядка байт в 16 битной знаковой переменной.
     * @see template<typename T> bswap(T x);
     * @since 1.1
     */
    template<>
    inline int16_t bswap<int16_t>(int16_t x) {
        return 
        (((x & 0xff00) >> 8)) |
        (((x & 0x00ff) << 8)); 
    }

    /**
     * Оптимизированная версия функции для изменения порядка байт в 32 битной знаковой переменной.
     * @see template<typename T> bswap(T x);
     * @since 1.1
     */
    template<>
    inline int32_t bswap<int32_t>(int32_t x) {
        return 
            bswap<uint16_t>(((x >> 16) & 0xffff)) | 
            bswap<uint16_t>((x & 0xffff)) << 16;
    }

    /**
     * Оптимизированная версия функции для изменения порядка байт в 64 битной знаковой переменной.
     * @see template<typename T> bswap(T x);
     * @since 1.1
     */
    template<>
    inline int64_t bswap<int64_t>(int64_t x) {
        return 
        (int64_t) bswap<uint32_t>((int32_t) ((x >> 32) & 0xff'ff'ff'ff)) | 
        (int64_t) bswap<uint32_t>((int32_t) (x & 0xff'ff'ff'ff)) << 32;
    }

    /**
     * Оптимизированная версия функции для изменения порядка байт float переменной.
     * @see template<typename T> bswap(T x);
     * @since 1.1
     */
    template<>
    inline float bswap(float x) {
    #if __SIZEOF_FLOAT__ == 4
        uint32_t* v = reinterpret_cast<uint32_t*>(&x);
        *v = bswap<uint32_t>(*v);
        return *reinterpret_cast<float*>(v);
    #else
    #   error float not 32-bit
    #endif
    }

    /**
     * Оптимизированная версия функции для изменения порядка байт double переменной.
     * @see template<typename T> bswap(T x);
     * @since 1.1
     */
    template<>
    inline double bswap(double x) {
    #if __SIZEOF_DOUBLE__ == 8
        uint64_t* v = reinterpret_cast<uint64_t*>(&x);
        *v = bswap<uint64_t>(*v);
        return *reinterpret_cast<double*>(v);
    #else
    #   error double not 64-bit
    #endif
    }

    /**
     * Копирует блок памяти с изменением порядка байт в каждом элементе.
     *
     * Функция копирует данные из исходного буфера в целевой буфер,
     * меняя порядок байт в каждом элементе типа T.
     *
     * @tparam T 
     *      Тип элементов, для которых меняется порядок байт.
     * 
     * @param dst 
     *      Указатель на буфер в который будет происходить копирование.
     * 
     * @param src 
     *      Указатель на буфер из которого будет происходить копирование.
     * 
     * @param n 
     *      Количество элементов типа T.
     * 
     * @since 1.1
     * 
     * @IntrinsicCandidate
     */
    template<typename T>
    void copy_swap_memory(void* dst, const void* src, std::size_t n) {
        T tmp;
        unsigned char* dest         = reinterpret_cast<unsigned char*>(dst);
        const unsigned char* source = reinterpret_cast<const unsigned char*>(src);
        for (std::size_t i = 0; i < n; ++i) {
            std::memcpy(&tmp, source, sizeof(T));
            tmp = bswap<T>(tmp);
            std::memcpy(dest, &tmp, sizeof(T));
            dest    += sizeof(T);
            source  += sizeof(T);
        }
    }

    /**
     * Читает значение из указателя с преобразованием порядка байт.
     * 
     * @warning Данная функция работает только с примитивами!
     * 
     * Для типов размером больше 1 байта выполняет преобразование порядка байт,
     * если целевой порядок (out_order) не совпадает с порядком системы.
     * Для однобайтовых типов возвращает значение без изменений.
     * 
     * @tparam T 
     *      Тип читаемого значения.
     * 
     * @param ptr 
     *      Указатель на читаемое значение.
     * 
     * @param out_order 
     *      Порядок байт возвращаемого значения.
     * 
     * @return 
     *      Прочитанное значение, преобразованное в передаваемый порядок байт.
     * 
     * @since 1.0
     * 
     * @IntrinsicCandidate
     */
    template<typename T>
    T read_with_order(const void* ptr, byte_order out_order) {
        static_assert(jstd::is_primitive<T>::value, "T must be primitive");
        T v;
        std::memcpy(&v, ptr, sizeof(T));
        if (out_order != system::native_byte_order())
            v = bswap<T>(v);
        return v;
    }
    
    /**
     * Записывает значение по указателю с преобразованием порядка байт.
     * 
     * @warning Данная функция работает только с примитивами!
     * 
     * Для типов размером больше 1 байта выполняет преобразование порядка байт,
     * если целевой порядок (out_order) не совпадает с порядком системы.
     * Для однобайтовых типов записывает значение без изменений.
     * 
     * @tparam T 
     *      Тип записываемого значения.
     * 
     * @param ptr 
     *      Указатель, по которому будет записано значение.
     * 
     * @param v 
     *      Значение для записи.
     * 
     * @param out_order 
     *      Порядок байт для записи.
     * 
     * @since 1.0
     * 
     * @IntrinsicCandidate
     */
    template<typename T>
    void write_with_order(void* ptr, T v, byte_order out_order) {
        static_assert(jstd::is_primitive<T>::value, "T must be primitive");
        if (out_order != jstd::system::native_byte_order())
            v = jstd::utils::bswap<T>(v);
        std::memcpy(ptr, &v, sizeof(T));
    }

    /**
     * Выполняет сортировку вставками с использованием пользовательского компаратора.
     *
     * @tparam T 
     *      Тип элементов массива.
     * 
     * @tparam T_COMPARATOR 
     *      Тип компаратора. По умолчанию — compare_to<T>, должен реализовывать operator()(const T&, const T&) и возвращать int (<0, 0, >0).
     *
     * @param array 
     *      Указатель на массив элементов, которые требуется отсортировать.
     * 
     * @param len 
     *      Количество элементов в массиве. Должно быть >= 0.
     *
     * @throws null_pointer_exception 
     *      Eсли array равен nullptr.
     * 
     * @throws illegal_argument_exception 
     *      Eсли len < 0.
     *
     * Сортирует массив вставками, используя заданный компаратор.
     * 
     * @since 1.0
     */
    template<typename T, typename T_COMPARATOR = compare_to<T>>
    void intersect_sort(T* array, int64_t len) {
#ifndef NDEBUG
    if (array == nullptr) throw_except<null_pointer_exception>("array must be != null");
    if (len < 0) throw_except<illegal_argument_exception>("len must be >= 0");
#endif//NDEBUG
        T_COMPARATOR compare;
        for (int64_t i = 1; i < len; ++i) {
            int64_t j = i;
            while (j > 0) {
                T& a = array[j - 1];
                T& b = array[j];
                int comp = compare(a, b);
                if (comp > 0) {
                    T tmp   = std::move(a);
                    a       = std::move(b);
                    b       = std::move(tmp);
                    --j;
                    continue;
                }
                break;
            }
        }
    }

namespace internal {
    template<typename T, typename T_COMPARATOR>
    void quick_sort_impl(T* array, int64_t left, int64_t right, T_COMPARATOR& compare) {
        if (left >= right) return;

        int64_t i = left;
        int64_t j = right;
        T& pivot = array[(left + right) / 2];

        while (i <= j) {
            while (compare(array[i], pivot) < 0) ++i;
            while (compare(array[j], pivot) > 0) --j;

            if (i <= j) {
                if (i != j) {
                    T tmp = std::move(array[i]);
                    array[i] = std::move(array[j]);
                    array[j] = std::move(tmp);
                }
                ++i;
                --j;
            }
        }

        if (left < j)   quick_sort_impl(array, left, j, compare);
        if (i < right)  quick_sort_impl(array, i, right, compare);
    }
}

    /**
     * Выполняет быструю сортировку массива с использованием пользовательского компаратора.
     *
     * @tparam T 
     *      Тип элементов массива.
     * 
     * @tparam T_COMPARATOR 
     *      Тип компаратора. По умолчанию — compare_to<T>.
     *
     * @param array 
     *      Указатель на массив, который нужно отсортировать.
     * 
     * @param len 
     *      Количество элементов в массиве. Должно быть >= 0.
     *
     * @throws null_pointer_exception 
     *      Eсли array == nullptr
     * 
     * @throws illegal_argument_exception 
     *      Eсли len < 0
     * 
     * @since 1.0
     */
    template<typename T, typename T_COMPARATOR = compare_to<T>>
    void quick_sort(T* array, int64_t len) {
    #ifndef NDEBUG
        if (array == nullptr) throw_except<null_pointer_exception>("array must be != null");
        if (len < 0) throw_except<illegal_argument_exception>("len must be >= 0");
    #endif//NDEBUG
        if (len <= 1) return;
        T_COMPARATOR compare;
        internal::quick_sort_impl(array, 0, len - 1, compare);
    }
}

}

#endif//_ALLOCATORS_UTILS_H_