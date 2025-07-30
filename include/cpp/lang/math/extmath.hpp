#ifndef JSTD_CPP_LANG_MATH_EXT_MATH_H
#define JSTD_CPP_LANG_MATH_EXT_MATH_H

#include <cpp/lang/math/math.hpp>
#include <cpp/lang/math/mat4.hpp>
#include <cpp/lang/math/vec4.hpp>
#include <cpp/lang/math/vec3.hpp>
#include <cpp/lang/math/quat.hpp>

namespace jstd {
namespace math {

    inline mat4 scale(
        math::internal::real_t x, 
        math::internal::real_t y, 
        math::internal::real_t z) {
        return mat4(
                            x, 0, 0, 0, 
                            0, y, 0, 0,
                            0, 0, z, 0,
                            0, 0, 0, 1
                        );
    }

    template<typename VEC>
    mat4 scale(const VEC& v) {
        return scale(v.x, v.y, v.z);
    }

    inline mat4 identity() {
        return scale(1, 1, 1);
    }

    inline mat4 translate(
        math::internal::real_t x, 
        math::internal::real_t y, 
        math::internal::real_t z) {
        return mat4(
                            1, 0, 0, x, 
                            0, 1, 0, y,
                            0, 0, 1, z,
                            0, 0, 0, 1
                        );
    }

    template<typename VEC>
    mat4 translate(const VEC& v) {
        return translate(v.x, v.y, v.z);
    }
    
    inline mat4 x_rotate(math::internal::real_t rad) {
        using jstd::math::internal::real_t;
        real_t s = sin(rad);
        real_t c = cos(rad);
        return mat4(
                            1,  0,  0,  0, 
                            0,  c, -s, 0,
                            0,  s,  c,  0,
                            0,  0,  0,  1
                        );
    }

    inline mat4 y_rotate(math::internal::real_t rad) {
        using jstd::math::internal::real_t;
        real_t s = sin(rad);
        real_t c = cos(rad);
        return mat4(
            c,  0,  s,  0, 
            0,  1,  0,  0,
           -s,  0,  c,  0,
            0,  0,  0,  1
        );
    }

    inline mat4 z_rotate(math::internal::real_t rad) {
        using jstd::math::internal::real_t;
        real_t s = sin(rad);
        real_t c = cos(rad);
        return mat4(
            c, -s,  0,  0, 
            s,  c,  0,  0,
            0,  0,  1,  0,
            0,  0,  0,  1
        );
    }

    inline mat4 frustum(
        math::internal::real_t left, 
        math::internal::real_t right, 
        math::internal::real_t bottom, 
        math::internal::real_t top, 
        math::internal::real_t near, 
        math::internal::real_t far) {
        using math::internal::real_t;

        const real_t _00    = (2 * near)            /   (right -  left);
        const real_t _11    = (2 * near)            /   (top - bottom);
        const real_t A      = (right + left)        /   (right - left);
        const real_t B      = (top + bottom)        /   (top - bottom);
        const real_t C      = -((far + near)        /   (far - near));
        const real_t D      = -((2 * far * near)    /   (far - near));

        return mat4(
            _00, 0,     A,      0, 
            0,  _11,    B,      0,
            0,   0,     C,      D,
            0,   0,    -1,      0
        );
    }

    inline mat4 perspective(
        math::internal::real_t fov, 
        math::internal::real_t aspect, 
        math::internal::real_t near, 
        math::internal::real_t far) {
        using math::internal::real_t;

        const real_t half_fov_tan = tan(fov / 2);
        const real_t _00 = 1.0 / (aspect * half_fov_tan);
        const real_t _11 = 1.0 / half_fov_tan;
        const real_t _22 = -((far + near) / (far - near));
        const real_t _23 = -((2 * far * near) / (far - near));

        return mat4(
            _00,    0,    0,    0, 
            0,     _11,   0,    0,
            0,      0,   _22,  _23,
            0,      0,   -1,    0
        );
    }
    
    inline mat4 ortho(
        math::internal::real_t left, 
        math::internal::real_t right, 
        math::internal::real_t bottom, 
        math::internal::real_t top, 
        math::internal::real_t near, 
        math::internal::real_t far) {
        using math::internal::real_t;
        
        const real_t _m00 = 2.0  / (right - left);
        const real_t _m11 = 2.0  / (top - bottom);
        const real_t _m22 = -2.0 / (far - near);

        const real_t tx = -((right + left) / (right - left));
        const real_t ty = -((top + bottom) / (top - bottom));
        const real_t tz = -((far + near)   / (far - near));

        return mat4(
            _m00,   0,      0,      tx, 
            0,      _m11,   0,      ty,
            0,      0,      _m22,   tz,
            0,      0,      0,      1
        );
    }

    inline quat from_equler_x(internal::real_t rad) {
        using internal::real_t;
        const real_t w = cos(rad * 0.5);
        const real_t x = sin(rad * 0.5);
        const real_t y = 0;
        const real_t z = 0;
        return quat(x, y, z, w);
    }

    inline quat from_equler_y(internal::real_t rad) {
        using internal::real_t;
        const real_t w = cos(rad * 0.5);
        const real_t x = 0;
        const real_t y = sin(rad * 0.5);
        const real_t z = 0;
        return quat(x, y, z, w);
    }

    inline quat from_equler_z(internal::real_t rad) {
        using internal::real_t;
        const real_t w = cos(rad * 0.5);
        const real_t x = 0;
        const real_t y = 0;
        const real_t z = sin(rad * 0.5);
        return quat(x, y, z, w);
    }

    inline double pow(double x, double power) {
        return ::pow(x, power);
    }

    inline long double pow(long double x, long double power) {
        return ::powl(x, power);
    }

    inline float pow(float x, float power) {
        return ::powf(x, power);
    }

    inline mat4 quat_to_matrix(const quat& q){
        using internal::real_t;
        const real_t x = q.x;
        const real_t y = q.y;
        const real_t z = q.z;
        const real_t w = q.w;

        const real_t _0 = 1.0 - 2.0 * (y*y + z*z);
        const real_t _1 = 2.0 * (x * y - w * z);
        const real_t _2 = 2.0 * (w * y + x * z);
        const real_t _3 = 0.0;
        
        const real_t _4 = 2.0 * (x * y + w * z);
        const real_t _5 = 1.0 - 2.0 * (x*x + z*z);
        const real_t _6 = 2.0 * (y * z - w * x);
        const real_t _7 = 0.0;

        const real_t _8  = 2.0 * (x * z - w * y);
        const real_t _9  = 2.0 * (w * x + y * z);
        const real_t _10 = 1.0 - 2.0 * (x*x + y*y);
        const real_t _11 = 0.0;
        
        const real_t _12 = 0.0;
        const real_t _13 = 0.0;
        const real_t _14 = 0.0;
        const real_t _15 = 1.0;

        return mat4(
                    _0, _1, _2, _3,
                    _4, _5, _6, _7,
                    _8, _9, _10,_11,
                    _12,_13,_14,_15
                );
    }

}//namespace math
}//namespace jstd
#endif//JSTD_CPP_LANG_MATH_EXT_MATH_H