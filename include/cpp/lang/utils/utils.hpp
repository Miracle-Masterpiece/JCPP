#ifndef _ALLOCATORS_UTILS_H_
#define _ALLOCATORS_UTILS_H_

#include <cstdint>
#include <utility>
#include <cpp/lang/system.hpp>
#include <cpp/lang/utils/comparator.hpp>
#include <cstring>

namespace jstd {

class null_pointer_exception;
class illegal_argument_exception;

template<typename T>
void throw_except(const char* format, ...);

namespace utils {

    void copy_swap_memory(void* dst, const void* src, std::size_t sz);
    void copy_swap_memory(void* dst, const void* src, std::size_t sz, std::size_t count);

    void swap(void* data, std::size_t sz);
    void swap(void* data, std::size_t sz, std::size_t n);

    template<typename T>
    inline T swap_bytes(T value) {
        swap(reinterpret_cast<void*>(&value), sizeof(value));
        return value;
    }

    template<typename T>
    T read_with_order(T* ptr, byte_order out_order) {
        T tmp = *ptr;
        if (sizeof(T) != sizeof(char) && system::native_byte_order() != out_order)
            swap(&tmp, sizeof(T));
        return tmp;
    }
    
    template<typename T>
    void write_with_order(T* ptr, T v, byte_order out_order) {
        if (sizeof(T) != sizeof(char) && system::native_byte_order() != out_order)
            swap(&v, sizeof(T));
        *ptr = v;
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