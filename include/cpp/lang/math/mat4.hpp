#ifndef CPP_LANG_MATH_MAT4_H
#define CPP_LANG_MATH_MAT4_H

#ifndef NDEBUG
#include <cpp/lang/exceptions.hpp>
#endif

#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/math/vec4.hpp>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace jstd {

template<typename T>
struct base_mat4;

typedef base_mat4<math::internal::real_t>   mat4;

typedef base_mat4<double>           mat4d;

typedef base_mat4<short>            mat4s;
typedef base_mat4<signed int>       mat4i;
typedef base_mat4<signed long>      mat4l;

typedef base_mat4<unsigned short>   mat4us;
typedef base_mat4<unsigned int>     mat4ui;
typedef base_mat4<unsigned long>    mat4ul;

typedef base_mat4<int8_t>   mat4i8;
typedef base_mat4<int16_t>  mat4i16;
typedef base_mat4<int32_t>  mat4i32;
typedef base_mat4<int64_t>  mat4i64;

typedef base_mat4<uint8_t>  mat4u8;
typedef base_mat4<uint16_t> mat4u16;
typedef base_mat4<uint32_t> mat4u32;
typedef base_mat4<uint64_t> mat4u64;

template<typename T>
struct base_mat4 {
    
    union {
        T m_data[16];
        T   m00, m01, m02, m03,
            m10, m11, m12, m13,
            m20, m21, m22, m23,
            m30, m31, m32, m33;
    };

    base_mat4(  T m00 = T(0), T m01 = T(0), T m02 = T(0), T m03 = T(0),
                T m10 = T(0), T m11 = T(0), T m12 = T(0), T m13 = T(0),
                T m20 = T(0), T m21 = T(0), T m22 = T(0), T m23 = T(0),
                T m30 = T(0), T m31 = T(0), T m32 = T(0), T m33 = T(0));
    base_mat4(const base_mat4<T>& m);
    base_mat4(base_mat4<T>&& m);
    base_mat4<T>& operator= (const base_mat4<T>& m);
    base_mat4<T>& operator= (base_mat4<T>&& m);
    ~base_mat4();

    base_mat4<T> transpose() const;
    
    bool equals(const base_mat4<T>&) const;
    bool operator== (const base_mat4<T>&) const;
    bool operator!= (const base_mat4<T>&) const;
    uint64_t hashcode() const;

    static const int32_t TO_STRING_MIN_BUFFER_SIZE = 224;
    int32_t to_string(char buf[], int32_t bufsize) const;

    base_mat4<T> add(const base_mat4<T>&) const;
    base_mat4<T> operator+(const base_mat4<T>&) const;

    base_mat4<T> sub(const base_mat4<T>&) const;
    base_mat4<T> operator-(const base_mat4<T>&) const;

    base_mat4<T> mul(const base_mat4<T>&) const;
    
    template<typename MT>
    friend base_vec4<MT> mat_mul_vec(const base_mat4<MT>&, const base_vec4<MT>&);

    template<typename MT>
    friend base_vec4<MT> vec_mul_mat(const base_vec4<MT>&, const base_mat4<MT>&);

    const T& get(int32_t row, int32_t col) const;
    T& get(int32_t row, int32_t col);

    base_mat4<T> operator*(const base_mat4<T>&) const;
};
    
    template<typename T>
    const T& base_mat4<T>::get(int32_t row, int32_t col) const {
        const int32_t y = row;
        const int32_t x = col;
        const int32_t idx = y + x * 4;
#ifndef NDEBUG
        check_index(idx, 16);
#endif
        return m_data[idx];
    }

    template<typename T>
    T& base_mat4<T>::get(int32_t row, int32_t col) {
        const int32_t y = row;
        const int32_t x = col;
        const int32_t idx = y + x * 4;
#ifndef NDEBUG
        check_index(idx, 16);
#endif
        return m_data[idx];
    }

    template<typename MT>
    base_vec4<MT> mat_mul_vec(const base_mat4<MT>& m, const base_vec4<MT>& v) {
        base_vec4<MT> result;
        for (int32_t y = 0; y < 4; ++y)
            for (int32_t x = 0; x < 4; ++x)
                result.get(y) += m.get(y, x) * v.get(x);
        return result;
    }

    template<typename MT>
    base_vec4<MT> vec_mul_mat(const base_vec4<MT>& v, const base_mat4<MT>& m) {
        base_vec4<MT> result;
         for (int32_t y = 0; y < 4; ++y)
            for (int32_t x = 0; x < 4; ++x)
                result.get(y) += v.get(x) * m.get(x, y);
        return result;
    }

    template<typename MT>
    base_vec4<MT> operator* (const base_mat4<MT>& m, const base_vec4<MT>& v) {
        return mat_mul_vec(m, v);
    }

    template<typename MT>
    base_vec4<MT> operator* (const base_vec4<MT>& v, const base_mat4<MT>& m) {
        return vec_mul_mat(v, m);
    }

    template<typename T>
    base_mat4<T>::base_mat4(    T m00, T m01, T m02, T m03,
                                T m10, T m11, T m12, T m13,
                                T m20, T m21, T m22, T m23,
                                T m30, T m31, T m32, T m33) {
        get(0,0) = m00; get(0,1) = m01; get(0,2) = m02; get(0,3) = m03;
        get(1,0) = m10; get(1,1) = m11; get(1,2) = m12; get(1,3) = m13;
        get(2,0) = m20; get(2,1) = m21; get(2,2) = m22; get(2,3) = m23;
        get(3,0) = m30; get(3,1) = m31; get(3,2) = m32; get(3,3) = m33;
    }
    
    template<typename T>
    base_mat4<T>::base_mat4(const base_mat4<T>& m) {
        std::memcpy(m_data, m.m_data, sizeof(T) * 16);
    }
    
    template<typename T>
    base_mat4<T>::base_mat4(base_mat4<T>&& m) {
        std::memcpy(m_data, m.m_data, sizeof(T) * 16);
    }
    
    template<typename T>
    base_mat4<T>& base_mat4<T>::operator= (const base_mat4<T>& m) {
        if (&m != this)
            std::memcpy(m_data, m.m_data, sizeof(T) * 16);
        return *this;
    }
    
    template<typename T>
    base_mat4<T>& base_mat4<T>::operator= (base_mat4<T>&& m) {
        if (&m != this)
            std::memcpy(m_data, m.m_data, sizeof(T) * 16);
        return *this;
    }
    
    template<typename T>
    base_mat4<T>::~base_mat4() {

    }

    template<typename T>
    base_mat4<T> base_mat4<T>::transpose() const {
        base_mat4<T> result = *this;
        int32_t idx = 0;
		for (int32_t x = 0; x < 4; ++x)
			for (int32_t y = 0; y < 16; y+= 4)
				result.m_data[idx++] = m_data[x + y];
        return result;
    }

    template<typename T>
    bool base_mat4<T>::equals(const base_mat4<T>& m) const {
        return objects::equals(m_data, m.m_data, 16);
    }
    
    template<typename T>
    bool base_mat4<T>::operator== (const base_mat4<T>& m) const {
        return equals(m);
    }
    
    template<typename T>
    bool base_mat4<T>::operator!= (const base_mat4<T>& m) const {
        return !equals(m);
    }
    
    template<typename T>
    uint64_t base_mat4<T>::hashcode() const {
        return objects::hashcode(m_data, 16);
    }

    template<typename T>
    int32_t base_mat4<T>::to_string(char buf[], int32_t bufsize) const {
        int32_t offset = 0;
        
        for (int32_t y = 0; y < 4; ++y) {
            const T v0 = get(y, 0);
            const T v1 = get(y, 1);
            const T v2 = get(y, 2);
            const T v3 = get(y, 3);
            if (bufsize <= offset)
                break;
            int32_t len = std::snprintf(buf + offset, bufsize - offset, "%g, %g, %g, %g\n", (double) v0, (double) v1, (double) v2, (double) v3);
            if (len == 0)
                break;
            offset += len;
        }   
    
        return offset;
    }


    template<typename T>
    base_mat4<T> base_mat4<T>::mul(const base_mat4<T>& m) const {
        base_mat4<T> r;
        /**
         * 0  1  2  3       0  1  2  3
         * 4  5  6  7       4  5  6  7
         * 8  9  10 11      8  9  10 11
         * 12 13 14 15      12 13 14 15
         */
        for (int32_t y = 0; y < 4; ++y) {
            for (int32_t x = 0; x < 4; ++x) {
                r.get(y, x) = 
                                get(y, 0) * m.get(0, x) +
                                get(y, 1) * m.get(1, x) +
                                get(y, 2) * m.get(2, x) +
                                get(y, 3) * m.get(3, x);
            }   
        }

        return r;
    }
    
    template<typename T>
    base_mat4<T> base_mat4<T>::operator*(const base_mat4<T>& m) const {
        return mul(m);
    }

    template<typename T>
    base_mat4<T> base_mat4<T>::add(const base_mat4<T>& m) const {
        base_mat4<T> result;
        for (int32_t i = 0; i < 4 * 4; ++i)
            result.m_data[i] = m_data[i] + m.m_data[i];
        return result;
    }
    
    template<typename T>
    base_mat4<T> base_mat4<T>::operator+(const base_mat4<T>& m) const {
        return add(m);
    }

    template<typename T>
    base_mat4<T> base_mat4<T>::sub(const base_mat4<T>& m) const {
        base_mat4<T> result;
        for (int32_t i = 0; i < 4 * 4; ++i)
            result.m_data[i] = m_data[i] - m.m_data[i];
        return result;
    }
    
    template<typename T>
    base_mat4<T> base_mat4<T>::operator-(const base_mat4<T>& m) const {
        return sub(m);
    }
}

#endif//CPP_LANG_MATH_MAT4_H