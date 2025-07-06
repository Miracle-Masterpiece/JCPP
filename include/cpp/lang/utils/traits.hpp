#ifndef _JSTD_CPP_LANG_UTILS_TRAITS_H_
#define _JSTD_CPP_LANG_UTILS_TRAITS_H_
namespace jstd {

/**
 * Метаструктура для проверки, является ли тип указателем.
 * 
 * Шаблон определяет, является ли тип T указателем. 
 * По умолчанию значение value устанавливается в false. 
 * Для специализаций шаблона (для обычных указателей и указателей на const) значение value будет установлено в true.
 * 
 * @tparam T 
 *      Тип, который проверяется на указатель.
 */
template<typename T>
struct is_pointer {
    static const bool value = false;
};

/**
 * Специализация для обычных указателей.
 * 
 * Специализация шаблона is_pointer для типов, являющихся указателями. 
 * Устанавливает значение value в true для обычных указателей.
 * 
 * @tparam T 
 *      Тип, являющийся указателем.
 */
template<typename T>
struct is_pointer<T*> {
    static const bool value = true;
};

/**
 * Специализация для указателей на const.
 * 
 * Специализация шаблона is_pointer для типов, являющихся указателями на const.
 * Устанавливает значение value в true для указателей на const.
 * 
 * @tparam T 
 *      Тип, являющийся указателем на const.
 */
template<typename T>
struct is_pointer<const T*> {
    static const bool value = true;
};

/**
 * Метаструктура для проверки, является ли тип примитивным.
 * 
 * Данный trait определяет, относится ли тип T к примитивным типам.
 * Примитивные типы включают:
 * - Целочисленные типы (char, int, long и т. д.).
 * - Числа с плавающей запятой (float, double).
 * - Указатели (T*).
 * - const версии этих типов.
 * 
 * @tparam T 
 *      Тип, который проверяется на примитивность.
 */
template<typename T>
struct is_primitive {
    /**
     * Значение, указывающее, является ли T примитивным типом.
     * 
     * По умолчанию проверяет, является ли T указателем (is_pointer<T>::value).
     * Если T — указатель, value будет true, иначе false.
     */
    static const bool value = is_pointer<T>::value;
};

/**
 * Макрос для создания специализаций `is_primitive<T>` для встроенных типов.
 * 
 * Макрос создаёт две специализации для каждого типа:
 * 1. is_primitive<type> — для обычной версии типа.
 * 2. is_primitive<const type> — для const версии типа.
 * 
 * Обе специализации устанавливают value = true, так как данные типы являются примитивными.
 * 
 * @param type 
 *      Тип, который должен быть помечен как примитивный.
 */
#define ___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(type)                  \
                    template<>                                          \
                    struct is_primitive<type> {                         \
                        static const bool value = true;                 \
                    };                                                  \
                    template<>                                          \
                    struct is_primitive<const type> {                   \
                        static const bool value = true;                 \
                    };

/**
 * Специализации is_primitive<T> для встроенных типов.
 * 
 * Определяет примитивными следующие типы:
 * - char, signed char, unsigned char
 * - short, int, long, long long (и их знаковые/беззнаковые версии)
 * - float, double
 * - const версии всех вышеуказанных типов
 */
___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(char)
___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(signed char)
___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(unsigned char)

___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(signed short)
___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(unsigned short)

___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(signed int)
___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(unsigned int)

___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(signed long)
___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(unsigned long)

___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(signed long long)
___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(unsigned long long)

___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(float)
___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(double)
___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__(long double)

/**
 * @undef ___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__
 * 
 * Удаляет макрос ___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__
 */
#undef ___JSTD__MAKE_PRIMITIVE_SPECIALIZATION__

/**
 * Метафункция для проверки эквивалентности типов.
 * 
 * is_same<T, D> позволяет определить, являются ли два типа T и D одинаковыми.
 * 
 * @tparam T 
 *      Первый тип для сравнения.
 * 
 * @tparam D 
 *      Второй тип для сравнения.
 */
template<typename T, typename D>
struct is_same {
    /**
     * Значение, указывающее, являются ли T и D одним и тем же типом.
     * 
     * По умолчанию устанавливается в false, если T и D различны.
     */
    static const bool value = false;
};

/**
 * Частичная специализация для случая, когда T и D совпадают.

 * Данная специализация переопределяет value в true, если T и D являются одним и тем же типом.
 * 
 * @tparam T 
 *      Тип, который сравнивается сам с собой.
 */
template<typename T>
struct is_same<T, T> {
    /**
     * Значение true, указывающее, что T и T — это один и тот же тип.
     */
    static const bool value = true;
};

/**
 * Метаструктура для удаления квалификатора const на верхнем уровне типа.
 *
 * Структура remove_const предоставляет тип type, в котором удалён квалификатор
 * const только на верхнем уровне типа T. 
 * 
 * Вложенные const, такие как в указателях (const int*) или ссылках (const int&), остаются неизменными.
 *
 * Эта реализация эквивалентна std::remove_const из стандартной библиотеки C++.
 *
 * @tparam T 
 *      Тип, из которого требуется удалить верхнеуровневый const.
 *
 * @note 
 *      Указатели и ссылки не модифицируются, только сам тип `T`.
 *
 * Примеры использования:
 * @code
 *      remove_const<const int>::type          // int
 *      remove_const<int>::type                // int
 *      remove_const<const int*>::type         // const int*
 *      remove_const<int* const>::type         // int*
 *      remove_const<const int&>::type         // const int& (не изменяется)
 * @endcode
 */
template<typename T>
struct remove_const {
    typedef T type;
};

/**
 * Частичная специализация remove_const для типов с const.
 *
 * Удаляет квалификатор `const` на верхнем уровне, оставляя вложенные const без изменений.
 *
 * @tparam T 
 *      Базовый тип, обёрнутый в const.
 */
template<typename T>
struct remove_const<const T> {
    typedef T type;
};

/**
 * Добавляет модификатор const к типу T.
 *
 * Шаблонная структура, эквивалентная std::add_const из стандартной библиотеки.
 * Применяется для получения const T на верхнем уровне, не изменяя структуру указателей или ссылок.
 *
 * Примеры:
 * - add_const<int>::type           ->              const int
 * - add_const<int*>::type          ->              int* const
 * - add_const<const int*>::type    ->              const int* const
 * - add_const<int&>::type          ->              int& 
 *
 * @tparam 
 *      T Тип, к которому необходимо добавить const.
 */
template<typename T>
struct add_const {
    typedef const T type;
};

/**
 * Базовая структура для представления "очищенного" типа.
 *
 * Содержит два typedef-а:
 * - type — тип без const, указателей и ссылок.
 * - const_type — const-версия очищенного типа.
 *
 * Используется как базовый класс для специализаций pure_type.
 *
 * @tparam T 
 *      Исходный тип.
 */
template<typename T>
struct base_pure_type {
    typedef T type;
    typedef const T const_type;
};

/**
 * Структура, проверяющая, относится ли тип DERIVED_T к типу BASE_T.
 * 
 * @tparam DERIVED_T
 *      Тип, который проверяется, принажлежит ли он к типу BASE_T.
 * 
 * @tparam BASE_T
 *      Базовый тип.
 */
template<typename DERIVED_T, typename BASE_T>
struct is_base_of {
private:
    static char test(const BASE_T* v);
    static long test(const void* v);
public:
    static const bool value = sizeof(test((const DERIVED_T*) 1)) == sizeof(char) ? true : false;
};

/**
 * Структура, очищающая тип от const, указателей, ссылок и rvalue-ссылок.
 *
 * Рекурсивно удаляет верхнеуровневые модификаторы:
 * - const
 * - указатели (*)
 * - ссылки (&)
 * - rvalue-ссылки (&&)
 *
 * Используется для извлечения "чистого" типа, пригодного для хранения, сравнения типов и других метаопераций.
 *
 * @tparam T 
 *      Исходный тип.
 */
template<typename T>
struct pure_type : base_pure_type<T> {};

/** Специализация: удаляет верхнеуровневый const */
template<typename T>
struct pure_type<const T> : base_pure_type<typename pure_type<T>::type> {};

/** Специализация: удаляет указатель */
template<typename T>
struct pure_type<T*> : base_pure_type<typename pure_type<T>::type> {};

/** Специализация: удаляет const указатель */
template<typename T>
struct pure_type<const T*> : base_pure_type<typename pure_type<T>::type> {};

/** Специализация: удаляет ссылку */
template<typename T>
struct pure_type<T&> : base_pure_type<typename pure_type<T>::type> {};

/** Специализация: удаляет rvalue-ссылку */
template<typename T>
struct pure_type<T&&> : base_pure_type<typename pure_type<T>::type> {};

template<typename T>
struct is_unsigned {
    static const bool value = false;
};

#define ___JSTD__MAKE_IS_UNSIGNED_SPECIALIZATION__(type)                \
                    template<>                                          \
                    struct is_unsigned<type> {                          \
                        static const bool value = true;                 \
                    };                                                  \
                    template<>                                          \
                    struct is_unsigned<const type> {                    \
                        static const bool value = true;                 \
                    };

___JSTD__MAKE_IS_UNSIGNED_SPECIALIZATION__(unsigned char);
___JSTD__MAKE_IS_UNSIGNED_SPECIALIZATION__(unsigned short);
___JSTD__MAKE_IS_UNSIGNED_SPECIALIZATION__(unsigned int);
___JSTD__MAKE_IS_UNSIGNED_SPECIALIZATION__(unsigned long);
___JSTD__MAKE_IS_UNSIGNED_SPECIALIZATION__(unsigned long long);

#undef ___JSTD__MAKE_IS_UNSIGNED_SPECIALIZATION__

}
#endif//_JSTD_CPP_LANG_UTILS_TRAITS_H_