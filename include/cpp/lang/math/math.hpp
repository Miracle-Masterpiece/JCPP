#ifndef JSTD_CPP_LANG_MATH_MATH_H
#define JSTD_CPP_LANG_MATH_MATH_H

#include <cmath>

#ifndef M_PI
    #define M_PI 3.141592653589793
#endif

#ifndef M_HALF_PI
    #define M_HALF_PI (3.141592653589793 / 2.0)
#endif

namespace jstd {
namespace math {

    template<typename T>
    constexpr T lerp(const T& a, const T& b, double t) {
        return (b - a) * t + a;
    }

    template<typename T>
    constexpr T lerp(const T& a, const T& b, float t) {
        return (b - a) * t + a;
    }

    inline double to_radians(double angle) {
        return angle * (M_PI / 180.0);
    }

    inline double to_degrees(double rad) {
        return rad * (180.0 / M_PI);
    }

    inline float to_radians(float angle) {
        return angle * (M_PI / 180.0);
    }

    inline float to_degrees(float rad) {
        return rad * (180.0 / M_PI);
    }

    inline long double to_radians(long double angle) {
        return angle * (M_PI / 180.0);
    }

    inline long double to_degrees(long double rad) {
        return rad * (180.0 / M_PI);
    }

    inline long double sin(long double x) {
        return ::sinl(x);
    }

    inline double sin(double x) {
        return ::sin(x);
    }

    inline float sin(float x) {
        return ::sinf(x);
    }
    
    inline long double cos(long double x) {
        return ::cosl(x);
    }

    inline double cos(double x) {
        return ::cos(x);
    }

    inline float cos(float x) {
        return ::cosf(x);
    }

    inline double tan(double x) {
        return ::tan(x);
    }

    inline long double tan(long double x) {
        return ::tanl(x);
    }
    
    inline float tan(float x) {
        return ::tanf(x);
    }

    inline double ctg(double x) {
        return cos(x) / sin(x);
    }

    inline long double ctg(long double x) {
        return cos(x) / sin(x);
    }

    inline float ctg(float x) {
        return cos(x) / sin(x);
    }

    template<typename T>
    inline T sqrt(T val) {
        return (T) ::sqrt((double) val);
    }

    template<>
    inline double sqrt<double>(double x) {
        return ::sqrt(x);
    }    

    template<>
    inline long double sqrt<long double>(long double x) {
        return ::sqrtl(x);
    }

    template<>
    inline float sqrt<float>(float x) {
        return ::sqrtf(x);
    }

    template<typename T>
    inline T abs(T x) {
        if (x < 0)
            return -x;
        else
            return x;
    }

    inline long double atan2(long double y, long double x) {
        return ::atan2l(y, x);
    }

    inline double atan2(double y, double x) {
        return ::atan2(y, x);
    }

    inline float atan2(float y, float x) {
        return ::atan2f(y, x);
    }
  
}//namespace math
}//namespace jstd

#endif//JSTD_CPP_LANG_MATH_MATH_H