#ifndef _JSTDLIB_CPP_LANG_EXCEPTIONS_THROWABLE_H_
#define _JSTDLIB_CPP_LANG_EXCEPTIONS_THROWABLE_H_

#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <utility>
#include <cpp/lang/utils/traits.hpp>
#include <cpp/lang/stacktrace/stacktrace.hpp>

#define JSTD_THROWABLE_CAUSE_SIZE 256

namespace jstd 
{

class throwable {
    calltrace m_calltrace;
    char _cause[JSTD_THROWABLE_CAUSE_SIZE];
public:
    throwable();    
    throwable(const char* cause);
    throwable(const throwable& t);
    throwable(throwable&& t);
    throwable& operator= (const throwable& t);
    throwable& operator= (throwable&& t);
    virtual ~throwable();
    virtual int to_string(char buf[], int bufsize) const;
    const char* cause() const;
    void print_stack_trace() const;
    const calltrace& get_calltrace() const;
    void set_calltrace(calltrace&& calltrace);
};

#define ___MAKE_EXCEPT_CLASS__(clazz_name, _extends)                                                    \
                                                                                                        \
                                                class clazz_name : public _extends {                    \
                                                public:                                                 \
                                                    clazz_name();                                       \
                                                    clazz_name(const char* cause);                      \
                                                    clazz_name(const clazz_name& t);                    \
                                                    clazz_name(clazz_name&& t);                         \
                                                    clazz_name& operator= (const clazz_name& t);        \
                                                    clazz_name& operator= (clazz_name&& t);             \
                                                    ~clazz_name();                                      \
                                                };                                                      \


___MAKE_EXCEPT_CLASS__(error,                               throwable)
___MAKE_EXCEPT_CLASS__(out_of_memory_error,                 error)
___MAKE_EXCEPT_CLASS__(exception,                           throwable)
___MAKE_EXCEPT_CLASS__(runtime_exception,                   exception)
___MAKE_EXCEPT_CLASS__(null_pointer_exception,              runtime_exception)
___MAKE_EXCEPT_CLASS__(sequrity_exception,                  runtime_exception)
___MAKE_EXCEPT_CLASS__(interrupted_exception,               exception)


/**
 * #################################################################
 *                  R A N G E  E X C E P T I O N S
 * #################################################################
 */
___MAKE_EXCEPT_CLASS__(index_out_of_bound_exception,        runtime_exception)
___MAKE_EXCEPT_CLASS__(overflow_exception,                  runtime_exception)
___MAKE_EXCEPT_CLASS__(underflow_exception,                 runtime_exception)
___MAKE_EXCEPT_CLASS__(no_such_element_exception,           runtime_exception)
___MAKE_EXCEPT_CLASS__(stack_overflow_error,                error)

/**
 * #################################################################
 *                  S T A T E  E X C E P T I O N S
 * #################################################################
 */
___MAKE_EXCEPT_CLASS__(illegal_argument_exception,          runtime_exception)
___MAKE_EXCEPT_CLASS__(illegal_state_exception,             runtime_exception)
___MAKE_EXCEPT_CLASS__(unsupported_operation_exception,             exception)
___MAKE_EXCEPT_CLASS__(utf_format_exception,                runtime_exception)
___MAKE_EXCEPT_CLASS__(invalid_data_format_exception,       runtime_exception)
                
/**
 * #################################################################
 *                  I O E X C E P T I O N S
 * #################################################################
 */
___MAKE_EXCEPT_CLASS__(io_exception,                        exception)
___MAKE_EXCEPT_CLASS__(file_not_found_exception,            io_exception)
___MAKE_EXCEPT_CLASS__(eof_exception,                       io_exception)
___MAKE_EXCEPT_CLASS__(closed_exception,                    io_exception)
___MAKE_EXCEPT_CLASS__(readonly_exception,                  io_exception)

//socket exceptions
___MAKE_EXCEPT_CLASS__(socket_exception,                    io_exception)
___MAKE_EXCEPT_CLASS__(unknow_host_exception,               io_exception)
___MAKE_EXCEPT_CLASS__(connect_exception,                   socket_exception)
___MAKE_EXCEPT_CLASS__(bind_exception,                      socket_exception)
___MAKE_EXCEPT_CLASS__(socket_timeout_exception,            socket_exception)

#undef ___MAKE_EXCEPT_CLASS__


    template<typename T>
    T make_except(const char* format, ...) {
        va_list args;
        char buf[JSTD_THROWABLE_CAUSE_SIZE];
        va_start(args, format);
        std::vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        return T(buf);
    }

    template<typename T>
    void throw_except(const char* format, ...) {
        va_list args;
        char buf[JSTD_THROWABLE_CAUSE_SIZE];
        va_start(args, format);
        std::vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        T except(buf);
        except.set_calltrace(calltrace::current());
        throw T(std::move(except));
    }

    template<typename T>
    void throw_except() {
        throw T();
    }
    
    inline void check_non_null(const void* p, const char* msg) {
        if (p == nullptr)
            throw_except<null_pointer_exception>(msg);
    }
    
    inline void check_non_null(const void* p) {
        if (p == nullptr)
            throw_except<null_pointer_exception>();
    }

    template<typename T>
    void check_index(T idx, T len) {
        JSTD_DEBUG_CODE(
            if (is_unsigned<T>::value) {
                if (idx >= len)
                    throw_except<index_out_of_bound_exception>("Index %llu out of bound for length %llu!", (unsigned long long) idx, (unsigned long long) len);
            } else {
                if (idx < 0 || idx >= len)
                    throw_except<index_out_of_bound_exception>("Index %d out of bound for length %d!", (signed long long) idx, (signed long long) len);
            }
        );
    }

}
#endif