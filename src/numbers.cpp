#include <cpp/lang/numbers.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cstring>

namespace jstd 
{

namespace num 
{
    int32_t float_to_int_bits(float v) {
        static_assert(sizeof(v) == sizeof(int32_t), "float must be 32_bit");
        int32_t value;
        std::memcpy(&value, &v, sizeof(float));
        return value;
    }
    
    int64_t double_to_long_bits(double v) {
        static_assert(sizeof(v) == sizeof(int64_t), "double must be 64_bit");
        int64_t value;
        std::memcpy(&value, &v, sizeof(double));
        return value;
    }

namespace internal 
{
        int8_t hex_to_dec(char c) {
            if (c >= '0' && c <= '9') {
                return c - '0';
            } else if (c >= 'a' && c <= 'f') {
                return c - 'a' + 10;
            } else if (c >= 'A' && c <= 'F') {
                return c - 'A' + 10;
            }
            return 0;
        }
        int64_t parse_hex_int(const char* value) {
            int64_t result = 0;
            bool is_negative = false;
            std::size_t str_len = std::strlen(value);
            std::size_t digit   = 1;
            while (str_len > 0) {
                --str_len;
                if (value[str_len] == '-') {
                    if (is_negative)
                        throw_except<number_format_exception>("%s illegal number", value);
                    is_negative = true;
                    continue;
                }    
                if (!is_hex_digit(value[str_len]))
                    throw_except<number_format_exception>("%s illegal number", value);
                result += hex_to_dec(value[str_len]) * digit;
                digit *= 16;
            }
            return is_negative ? -result : result;
        }
}//namespace internal

    bool is_hex_digit(char c) {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    bool is_digit(char c) {
        return c >= '0' && c <= '9';
    }

    int8_t char_to_digit(char c) {
        return c - '0';
    }

    int64_t parse_int(const char* value, int radix) {
        JSTD_DEBUG_CODE(
            check_non_null(value);
            if (radix != 2 && radix != 8 && radix != 10 && radix != 16)
                throw_except<illegal_argument_exception>("radix %d is illegal!", radix);
        );
        if (radix == 16)
            return internal::parse_hex_int(value);
        int64_t result = 0;
        bool is_negative = false;
        std::size_t str_len = std::strlen(value);
        std::size_t digit   = 1;
        while (str_len > 0) {
            --str_len;
            if (value[str_len] == '-') {
                if (is_negative)
                    throw_except<number_format_exception>("%s illegal number", value);
                is_negative = true;
                continue;
            }
            if (value[str_len] < '0' || value[str_len] > '0' + (radix - 1))
                throw_except<number_format_exception>("%s illegal number", value);
            result += (value[str_len] - '0') * digit;
            digit *= radix;
        }
        return is_negative ? -result : result;
    }

    float parse_float(const char* value) {
        return parse_double(value);
    }

    double parse_double(const char* value) {
        double result = 0;
        bool is_negative = false;
        std::size_t str_len = std::strlen(value);
        
        std::size_t dot_index = str_len;
        for (std::size_t i = 0; i < str_len; ++i) {
            if (value[i] == '.') {
                dot_index = i;
                break;
            }
        }
        {
            double digit  = 1;
            std::size_t i = dot_index;
            while (i > 0) {
                --i;
                if (value[i] == '-') {
                    if (is_negative)
                        throw_except<number_format_exception>("%s illegal number", value);
                    is_negative = true;
                    continue;
                } 
                if (value[i] == '.')
                    throw_except<number_format_exception>("%s illegal number", value);
                result += char_to_digit(value[i]) * digit;
                digit *= 10;
            }
        }
        {
            double digit;
            double multiplier;
            if (dot_index != str_len) {
                multiplier  = 0.1;
                digit       = 0.1;
            } else {
                multiplier  = 10;
                digit       = 1;
            }

            std::size_t i = dot_index + 1;
            while (i < str_len) {
                if (value[i] == '.' || !is_digit(value[i]))
                    throw_except<number_format_exception>("%s illegal number", value);
                result += char_to_digit(value[i]) * digit;
                digit *= multiplier;
                i++;
            }
        }
        return is_negative ? -result : result;
    }

}//namespace num

}//namespace jstd

