#ifndef JSTD_CPP_LANG_MATH_VEC3_H
#define JSTD_CPP_LANG_MATH_VEC3_H


#ifndef NDEBUG
    #include <cpp/lang/exceptions.hpp>
#endif

#include <cstdint>
#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/math/math.hpp>
#include <type_traits>
#include <internal/math_defs.hpp>

namespace jstd {

template<typename T>
struct base_vec3;

typedef base_vec3<math::internal::real_t> vec3;
typedef base_vec3<double>       vec3d;

typedef base_vec3<short>        vec3s;
typedef base_vec3<signed int>   vec3i;
typedef base_vec3<signed long>  vec3l;

typedef base_vec3<unsigned short>   vec3us;
typedef base_vec3<unsigned int>     vec3ui;
typedef base_vec3<unsigned long>    vec3ul;

typedef base_vec3<int8_t>   vec3i8;
typedef base_vec3<int16_t>  vec3i16;
typedef base_vec3<int32_t>  vec3i32;
typedef base_vec3<int64_t>  vec3i64;

typedef base_vec3<uint8_t>  vec3u8;
typedef base_vec3<uint16_t> vec3u16;
typedef base_vec3<uint32_t> vec3u32;
typedef base_vec3<uint64_t> vec3u64;

/**
 * Обобщённый трёхкомпонентный вектор.
 * 
 * Представляет собой структуру, содержащую три компоненты — x, y, z — и предоставляет
 * базовые арифметические, векторные и служебные операции. 
 * 
 * Поддерживает доступ к данным как по координатам (x, y, z), так и по цветовому представлению (r, g, b).
 * 
 * @tparam T 
 *      Тип компонентов вектора (например, float, double, int).
 */
template<typename T>
struct base_vec3 {

    /**
     * Компоненты вектора по координатам: x, y, z.
     */
    union {
        struct {
            T x, y, z;
        };
        /**
         * Альтернативное именование компонентов как цвета: r, g, b.
         */
        struct {
            T r, g, b;
        };
        /**
         * Массив из трёх компонентов.
         */
        T m_data[3];
    };

    /**
     * Конструктор по значениям компонентов.
     * 
     * @param x Значение X-компоненты.
     * @param y Значение Y-компоненты.
     * @param z Значение Z-компоненты.
     */
    base_vec3(const T& x = T(), const T& y = T(), const T& z = T());

    /**
     * Конструктор копирования.
     * 
     * @param other 
     *      Вектор, который будет скопирован.
     */
    base_vec3(const base_vec3<T>& other);

    /**
     * Конструктор перемещения.
     * 
     * @param other 
     *      Вектор, из которого будут перемещены данные.
     */
    base_vec3(base_vec3<T>&& other);

    /**
     * Оператор копирующего присваивания.
     * 
     * @param other 
     *      Вектор для копирования.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    base_vec3<T>& operator=(const base_vec3<T>& other);

    /**
     * Оператор перемещающего присваивания.
     * 
     * @param other 
     *      Вектор для перемещения.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    base_vec3<T>& operator=(base_vec3<T>&& other);

    /**
     *
     */
    ~base_vec3();

    /**
     * Сложение с другим вектором.
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Сумма двух векторов.
     */
    base_vec3<T> add(const base_vec3<T>& other) const;

    /**
     * Оператор сложение с другим вектором.
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Сумма двух векторов.
     */
    base_vec3<T> operator+(const base_vec3<T>& other) const;

    /**
     * Сложение с числом (покомпонентно).
     * 
     * @param scalar 
     *      Скалярное значение.
     * 
     * @return 
     *      Результат сложения.
     */
    base_vec3<T> add(const T& scalar) const;

    /**
     * Оператор сложение с числом (покомпонентно).
     * 
     * @param scalar 
     *      Скалярное значение.
     * 
     * @return 
     *      Результат сложения.
     */
    base_vec3<T> operator+(const T& scalar) const;

    /**
     * Вычитание другого вектора.
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Разность двух векторов.
     */
    base_vec3<T> sub(const base_vec3<T>& other) const;

    /**
     * Оператор вычитания другого вектора.
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Разность двух векторов.
     */
    base_vec3<T> operator-(const base_vec3<T>& other) const;

    /**
     * Вычитание скаляра (покомпонентно).
     */
    base_vec3<T> sub(const T& scalar) const;

    /**
     * Оператор вычитания скаляра.
     */
    base_vec3<T> operator-(const T& scalar) const;

    /**
     * Умножение на вектор (покомпонентное).
     */
    base_vec3<T> mul(const base_vec3<T>& other) const;

    /**
     * Оператор покомпонентного умножения.
     */
    base_vec3<T> operator*(const base_vec3<T>& other) const;

    /**
     * Умножение на скаляр.
     */
    base_vec3<T> mul(const T& scalar) const;

    /**
     * Оператор умножения на скаляр.
     */
    base_vec3<T> operator*(const T& scalar) const;

    /**
     * Деление на вектор (покомпонентное).
     */
    base_vec3<T> div(const base_vec3<T>& other) const;

    /**
     * Оператор покомпонентного деления.
     */
    base_vec3<T> operator/(const base_vec3<T>& other) const;

    /**
     * Деление на скаляр.
     */
    base_vec3<T> div(const T& scalar) const;

    /**
     * Оператор деления на скаляр.
     */
    base_vec3<T> operator/(const T& scalar) const;

    /**
     * Доступ к компоненте по индексу.
     * 
     * @param index 
     *      Индекс (0 - x, 1 - y, 2 - z).
     * 
     * @return 
     *      Ссылка на компоненту.
     */
    T& operator[](int32_t index);

    /**
     * Доступ к компоненте по индексу. (Константный)
     * 
     * @param index 
     *      Индекс (0 - x, 1 - y, 2 - z).
     * 
     * @return 
     *      Ссылка на компоненту.
     */
    const T& operator[](int32_t index) const;

    /**
     * Доступ к компоненте по индексу.
     * 
     * @param index 
     *      Индекс (0 - x, 1 - y, 2 - z).
     * 
     * @return 
     *      Ссылка на компоненту.
     */
    T& get(int32_t index);

    /**
     * Доступ к компоненте по индексу. (Константный)
     * 
     * @param index 
     *      Индекс (0 - x, 1 - y, 2 - z).
     * 
     * @return 
     *      Ссылка на компоненту.
     */
    const T& get(int32_t index) const;
    
    /**
     * Скалярное произведение (dot product).
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Скалярное произведение.
     */
    T dot(const base_vec3<T>& other) const;

    /**
     * Векторное произведение (cross product).
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Вектор, перпендикулярный двум исходным.
     */
    base_vec3<T> cross(const base_vec3<T>& other) const;

    /**
     * Нормализация вектора.
     * 
     * @return 
     *      Единичный вектор с тем же направлением.
     */
    base_vec3<T> normalize() const;

    /**
     * Длина (модуль) вектора.
     * 
     * @return 
     *      Длина вектора.
     */
    T length() const;

    /**
     * Проверка на равенство по компонентам.
     * 
     * @param other 
     *      Вектор для сравнения.
     * 
     * @return 
     *      true, если все компоненты равны.
     */
    bool equals(const base_vec3<T>& other) const;

    /**
     * Оператор сравнения на равенство.
     */
    bool operator==(const base_vec3<T>& other) const;

    /**
     * Оператор сравнения на неравенство.
     */
    bool operator!=(const base_vec3<T>& other) const;

    /**
     * Вычисление хеш-кода по компонентам.
     * 
     * @return 
     *      64-битное хеш-значение.
     */
    uint64_t hashcode() const;

    /**
     * Минимальный рекомендуемый размер буфера для to_string.
     */
    static const int32_t TO_STRING_MIN_BUFFER_SIZE = 48;

    /**
     * Преобразует вектор в строковое представление.
     * 
     * @param buf 
     *      Буфер, куда будет записана строка.
     * 
     * @param bufsize 
     *      Размер буфера.
     * 
     * @return 
     *      Количество записанных символов (не включая 0-терминатор).
     */
    int32_t to_string(char buf[], int32_t bufsize) const;

    /**
     * Оператор сложения с другим вектором (на месте).
     * 
     * Прибавляет компоненты другого вектора к текущему вектору.
     * 
     * @param other 
     *      Вектор, который прибавляется.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator +=(const base_vec3<T>& other);

    /**
     * Оператор вычитания другого вектора (на месте).
     * 
     * Вычитает компоненты другого вектора из текущего.
     * 
     * @param other 
     *      Вектор, который вычитается.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator -=(const base_vec3<T>& other);

    /**
     * Оператор поэлементного умножения с другим вектором (на месте).
     * 
     * Умножает соответствующие компоненты другого вектора на текущий.
     * 
     * @param other 
     *      Вектор, на который умножаются компоненты.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator *=(const base_vec3<T>& other);

    /**
     * Оператор поэлементного деления на другой вектор (на месте).
     * 
     * Делит компоненты текущего вектора на соответствующие компоненты другого.
     * 
     * @param other 
     *      Вектор, на компоненты которого происходит деление.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator /=(const base_vec3<T>& other);

    /**
     * Оператор прибавления скаляра ко всем компонентам вектора (на месте).
     * 
     * @param scalar 
     *      Значение, прибавляемое к каждой компоненте.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator +=(const T& scalar);

    /**
     * Оператор вычитания скаляра из всех компонентов вектора (на месте).
     * 
     * @param scalar 
     *      Значение, вычитаемое из каждой компоненты.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator -=(const T& scalar);

    /**
     * Оператор умножения всех компонентов вектора на скаляр (на месте).
     * 
     * @param scalar 
     *      Значение, на которое умножаются компоненты.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator *=(const T& scalar);

    /**
     * Оператор деления всех компонентов вектора на скаляр (на месте).
     * 
     * @param scalar 
     *      Значение, на которое делятся компоненты.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator /=(const T& scalar);

    /**
     * Префиксный инкремент (увеличение всех компонентов на 1).
     * 
     * @return 
     *      Ссылку на текущий вектор после инкремента.
     */
    base_vec3<T>& operator++();

    /**
     * Постфиксный инкремент (увеличение всех компонентов на 1).
     * 
     * @return 
     *      Копию вектора до инкремента.
     */
    base_vec3<T> operator++(int);

    /**
     * Префиксный декремент (уменьшение всех компонентов на 1).
     * 
     * @return 
     *      Ссылку на текущий вектор после декремента.
     */
    base_vec3<T>& operator--();

    /**
     * Постфиксный декремент (уменьшение всех компонентов на 1).
     * 
     * @return 
     *      Копию вектора до декремента.
     */
    base_vec3<T> operator--(int);

    /**
     * Унарный минус (инвертирует все компоненты вектора).
     *
     * @return
     *     Новый вектор с противоположными по знаку компонентами.
     */
    base_vec3<T> operator-() const;
};


    template<typename T>
    base_vec3<T>::base_vec3(const T& x, const T& y, const T& z) :
    x(x), y(y), z(z) {

    }
    
    template<typename T>
    base_vec3<T>::base_vec3(const base_vec3<T>& v) : 
    x(v.x), y(v.y), z(v.z) {

    }
    
    template<typename T>
    base_vec3<T>::base_vec3(base_vec3<T>&& v) : 
    x(v.x), y(v.y), z(v.z) {

    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator= (const base_vec3<T>& v) {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator= (base_vec3<T>&& v) {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>::~base_vec3() {

    }

    template<typename T>
    base_vec3<T> base_vec3<T>::add(const base_vec3<T>& v) const {
        return base_vec3<T>(x + v.x, y + v.y, z + v.z);
    }

    template<typename T>
    base_vec3<T> base_vec3<T>::operator+(const base_vec3<T>& v) const {
        return add(v);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::add(const T& s) const {
        return base_vec3<T>(x + s, y + s, z + s);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator+(const T& s) const {
        return add(s);
    }

    template<typename T>
    base_vec3<T> operator+(const T& s, const base_vec3<T>& v) {
        return v + s;
    }

    template<typename T>
    base_vec3<T> base_vec3<T>::sub(const base_vec3<T>& v) const {
        return base_vec3<T>(x - v.x, y - v.y, z - v.z);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator-(const base_vec3<T>& v) const {
        return sub(v);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::sub(const T& s) const {
        return base_vec3<T>(x - s, y - s, z - s);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator-(const T& s) const {
        return sub(s);
    }

    template<typename T>
    base_vec3<T> base_vec3<T>::mul(const base_vec3<T>& v) const {
        return base_vec3<T>(x * v.x, y * v.y, z * v.z);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator*(const base_vec3<T>& v) const {
        return mul(v);
    }
    
    template<typename T>
    base_vec3<T> operator*(const T& s, const base_vec3<T>& v) {
        return v * s;
    }

    template<typename T>
    base_vec3<T> base_vec3<T>::mul(const T& s) const {
        return base_vec3<T>(x * s, y * s, z * s);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator*(const T& s) const {
        return mul(s);
    }

    template<typename T>
    base_vec3<T> base_vec3<T>::div(const base_vec3<T>& v) const {
        return base_vec3<T>(x / v.x, y / v.y, z / v.z);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator/(const base_vec3<T>& v) const {
        return div(v);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::div(const T& s) const {
        return base_vec3<T>(x / s, y / s, z / s);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator/(const T& s) const {
        return div(s);
    }

    template<typename T>
    T& base_vec3<T>::operator[] (int32_t index) {
        return get(index);
    }
    
    template<typename T>
    const T& base_vec3<T>::operator[] (int32_t index) const {
        return get(index);
    }

    template<typename T>
    T& base_vec3<T>::get(int32_t index) {
#ifndef NDEBUG
        if (index > 2)
            throw_except<index_out_of_bound_exception>("Index %i out of bound for length 3", index);
#endif
        return m_data[index];
    }

    template<typename T>
    const T& base_vec3<T>::get(int32_t index) const {
#ifndef NDEBUG
        if (index > 2)
            throw_except<index_out_of_bound_exception>("Index %i out of bound for length 3", index);
#endif
        return m_data[index];
    }

    template<typename T>
    T base_vec3<T>::dot(const base_vec3<T>& v) const {
        return x * v.x + y * v.y + z * v.z;
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::cross(const base_vec3<T>& v) const {
        const T nx = (y * v.z) - (z * v.y);
        const T ny = (z * v.x) - (x * v.z);
        const T nz = (x * v.y) - (y * v.x);
        return base_vec3<T>(nx, ny, nz);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::normalize() const {
        const T len = length();
        return base_vec3<T>(x / len, y / len, z / len);
    }

    template<typename T>
    T base_vec3<T>::length() const {
        return (T) math::sqrt(x * x + y * y + z * z);
    }

    template<typename T>
    bool base_vec3<T>::equals(const base_vec3<T>& v) const {
        return x == v.x && v.y == v.y && z == v.z;
    }
    
    template<typename T>
    bool base_vec3<T>::operator== (const base_vec3<T>& v) const {
        return equals(v);
    }
    
    template<typename T>
    bool base_vec3<T>::operator!= (const base_vec3<T>& v) const {
        return !equals(v);
    }
    
    template<typename T>
    uint64_t base_vec3<T>::hashcode() const {
        return objects::hashcode(m_data, 3);
    }

    template<typename T>
    base_vec3<T>& base_vec3<T>::operator +=(const base_vec3<T>& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator -=(const base_vec3<T>& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator *=(const base_vec3<T>& v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator /=(const base_vec3<T>& v) {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }


    template<typename T>
    base_vec3<T>& base_vec3<T>::operator +=(const T& s) {
        x += s;
        y += s;
        z += s;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator -=(const T& s) {
        x -= s;
        y -= s;
        z -= s;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator *=(const T& s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator /=(const T& s) {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    template<typename T>
    int32_t base_vec3<T>::to_string(char buf[], int32_t bufsize) const {
        return std::snprintf(buf, bufsize, "[x=%g, y=%g, z=%g]", (double) x, (double) y, (double) z);
    }

    template<typename T>
    base_vec3<T>& base_vec3<T>::operator++() {
        ++x; ++y; ++z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator++(int) {
        base_vec3<T> old_value = *this;
        ++(*this);
        return old_value;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator--() {
        --x; --y; --z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator--(int) {
        base_vec3<T> old_value = *this;
        --(*this);
        return old_value;
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator-() const {
        return base_vec3<T>(-x, -y, -z);
    }
}
#endif//JSTD_CPP_LANG_MATH_VEC3_H