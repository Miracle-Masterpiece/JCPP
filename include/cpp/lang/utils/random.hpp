#ifndef JSTD_CPP_LANG_UTILS_RANDOM_H
#define JSTD_CPP_LANG_UTILS_RANDOM_H

#include <cstdint>
#include <cpp/lang/system.hpp>
#include <cpp/lang/utils/limits.hpp>
#include <cpp/lang/exceptions.hpp>

namespace jstd
{

/**
 * Генератор псевдослучайных чисел с возможностью задания начального зерна.
 * 
 * Основан на линейном конгруэнтном методе (LCG) с переменным приращением.
 * Используется два внутренних генератора {@code next0()} и {@code next1()},
 * обеспечивающих варьируемое поведение. Один и тот же seed гарантирует
 * детерминированную последовательность чисел.
 * 
 * Не является потокобезопасным. Для использования в многопоточной среде необходимо
 * синхронизировать доступ к объекту извне.
 */
class random {
    int64_t m_seed;

    /**
     * Внутренний генератор псевдослучайных чисел.
     * 
     * Реализован как линейный конгруэнтный генератор (LCG) по формуле:
     * {@code m_seed = m_seed * A + C}, где 
     *  A = 25214903917, 
     *  C — переменная величина,
     *  рассчитываемая как сумма двух вызовов {@code next1()}, взятая по модулю 4096.
     * 
     * @return 
     *      Следующее псевдослучайное 64-битное значение.
     */
    uint64_t next0();

    /**
     * Вспомогательный генератор, используемый в {@code next0()}.
     * Выполняет вычисление: {@code m_seed * 25214903917 + 17}.
     * 
     * @return 
     *      Промежуточное значение генерации.
     */
    uint64_t next1();

public:
    /**
     * Создаёт новый генератор случайных чисел с заданным или автоматически определённым зерном.
     * Если зерно не указано, используется текущее системное время в наносекундах.
     * 
     * @param seed 
     *      Начальное значение генератора.
     */
    random(int64_t seed = system::nano_time());

    /**
     * Конструктор копирования.
     * Копирует внутреннее состояние генератора.
     * 
     * @param other 
     *      Генератор, из которого копируется состояние.
     */
    random(const random& other);

    /**
     * Конструктор перемещения.
     * Перемещает внутреннее состояние генератора.
     * 
     * @param other 
     *      Генератор, состояние которого перемещается.
     */
    random(random&& other);

    /**
     * Оператор копирующего присваивания.
     * 
     * @param other 
     *      Генератор, из которого копируется состояние.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    random& operator=(const random& other);

    /**
     * Оператор перемещающего присваивания.
     * 
     * @param other 
     *      Генератор, из которого перемещается состояние.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    random& operator=(random&& other);

    /**
     */
    ~random();

    /**
     * Генерирует случайное значение указанного типа.
     * Для целых типов значение выбирается из полного диапазона типа {@code T}.
     * Для типов с плавающей точкой (float, double, long double) результат в диапазоне [0.0, 1.0).
     *
     * @tparam T 
     *      Тип возвращаемого значения.
     * 
     * @return 
     *      Случайное значение указанного типа.
     */
    template<typename T>
    T next();

    /**
     * Генерирует случайное значение типа T в диапазоне от 0 (включительно) до max (не включительно).
     * Значение {@code max} должно быть строго положительным.
     * 
     * @tparam T 
     *      Тип возвращаемого значения.
     * 
     * @param max 
     *      Верхняя граница диапазона (не включительно).
     * 
     * @return 
     *      Случайное значение в диапазоне [0, max).
     * 
     * @throws illegal_argument_exception 
     *      Eсли max < 0.
     */
    template<typename T>
    T next(const T max);

    /**
     * Заполняет указанный массив случайными значениями типа T.
     * Каждый элемент заполняется вызовом {@code next<T>()}.
     *
     * @tparam T 
     *      Тип элементов массива.
     * 
     * @param buf 
     *      Указатель на массив.
     * 
     * @param bufsize 
     *      Количество элементов, которые нужно заполнить.
     * 
     * @throws illegal_argument_exception 
     *      Eсли buf == nullptr
     *      Если bufsize < 0.
     */
    template<typename T>
    void values(T buf[], int32_t bufsize);

    /**
     * Заполняет указанный массив случайными значениями типа T в диапазоне [0, max).
     * Каждое значение генерируется вызовом {@code next<T>(max)}.
     * 
     * @tparam T 
     *      Тип элементов массива.
     * 
     * @param buf 
     *      Указатель на массив.
     * 
     * @param bufsize 
     *      Количество элементов, которые нужно заполнить.
     * 
     * @param max 
     *      Верхняя граница диапазона (не включительно).
     * 
     * @throws illegal_argument_exception 
     *      Eсли buf == nullptr
     *      Если bufsize < 0.
     */
    template<typename T>
    void values(T buf[], int32_t bufsize, T max);
};

    template<>
    float random::next();

    template<>
    double random::next();

    template<>
    long double random::next();

    template<typename T>
    T random::next() {
        return (T) next0();
    }

    template<typename T>
    T random::next(const T max) {
        JSTD_DEBUG_CODE(
            if (max < 0)
                throw_except<illegal_argument_exception>("invalid max value: %lli", (long long) max);
        );
        T v = (T) (next<T>() % max);
        return v < 0 ? -v : v;
    }

    template<typename T>
    void random::values(T buf[], int32_t bufsize) {
        JSTD_DEBUG_CODE(
            check_non_null(buf);
            if (bufsize < 0)
                throw_except<illegal_argument_exception>("invalid size: %lli", (long long) bufsize);
        );
        for (int32_t i = 0; i < bufsize; ++i)
            buf[i] = next<T>();
    }

    template<typename T>
    void random::values(T buf[], int32_t bufsize, T max) {
        JSTD_DEBUG_CODE(
            check_non_null(buf);
            if (bufsize < 0)
                throw_except<illegal_argument_exception>("invalid size: %lli", (long long) bufsize);
        );
        for (int32_t i = 0; i < bufsize; ++i)
            buf[i] = next<T>(max);
    }
}

#endif//JSTD_CPP_LANG_UTILS_RANDOM_H