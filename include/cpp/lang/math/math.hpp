#ifndef JSTD_CPP_LANG_MATH_MATH_H
#define JSTD_CPP_LANG_MATH_MATH_H

#include <cmath>
#include <cstdint>

#ifndef M_PI
    #define M_PI 3.141592653589793
#endif

#ifndef M_HALF_PI
    #define M_HALF_PI (3.141592653589793 / 2.0)
#endif

namespace jstd {
namespace math {

    template<typename T>
    constexpr T lerp(const T a, const T b, double t) {
        return (b - a) * t + a;
    }

    template<typename T>
    constexpr T lerp(const T a, const T b, float t) {
        return (b - a) * t + a;
    }

    constexpr double to_radians(double angle) {
        return angle * (M_PI / 180.0);
    }

    constexpr double to_degrees(double rad) {
        return rad * (180.0 / M_PI);
    }

    constexpr float to_radians(float angle) {
        return angle * (float) (M_PI / 180.0);
    }

    constexpr float to_degrees(float rad) {
        return rad * (float) (180.0 / M_PI);
    }

    constexpr long double to_radians(long double angle) {
        return angle * (M_PI / 180.0);
    }

    constexpr long double to_degrees(long double rad) {
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

    inline float asin(float x) {
        return ::asinf(x);
    }

    inline double asin(double x) {
        return ::asin(x);
    }

    inline long double asin(long double x) {
        return ::asinl(x);
    }

    inline float acos(float x) {
        return ::acosf(x);
    }

    inline double acos(double x) {
        return ::acos(x);
    }

    inline long double acos(long double x) {
        return ::acosl(x);
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

    inline uint64_t hash2d(int64_t x, int64_t y) {
        const uint64_t ux = (uint64_t) x;
        const uint64_t uy = (uint64_t) y;
        uint64_t hash = (ux * 0x0B00B135U) + (uy * 0x50FFC001U);
        hash ^= hash >> 13U;
        hash += hash & 832749825U;
        hash ^= hash >> 16U;
        return hash;
    }

    constexpr double quintic(double x) {
        return x * x * x * (x * (x * 6 - 15) + 10);
    }

    constexpr float quintic(float x) {
        return x * x * x * (x * (x * 6 - 15) + 10);
    }

    constexpr long double quintic(long double x) {
        return x * x * x * (x * (x * 6 - 15) + 10);
    }

    inline double cosine(double x) {
        return (1 - cos(M_PI * x)) / 2;
    }

    inline float cosine(float x) {
        return (1 - cos((float) M_PI * x)) / 2;
    }

    inline long double cosine(long double x) {
        return (1 - cos(M_PI * x)) / 2;
    }

    template<typename T>
    T min(const T& a, const T& b) {
        return a < b ? a : b;
    }

    template<typename T>
    T max(const T& a, const T& b) {
        return a > b ? a : b;
    }

    template<typename T>
    T abs(const T& x) {
        return x < 0 ? -x : x;
    }

    inline float floor(float x) {
        return ::floorf(x);
    }

    inline double floor(double x) {
        return ::floor(x);
    }

    inline long double floor(long double x) {
        return ::floorl(x);
    }

    template<typename T>
    T hypot(const T& x, const T& y) {
        return sqrt<T>(x*x + y*y);
    }

    /**
     * @return
     *      Псевдо-случайное значение в диапазоне [0, 1 (не включительно!)]
     */
    double random();

}//namespace math
}//namespace jstd

#endif//JSTD_CPP_LANG_MATH_MATH_H