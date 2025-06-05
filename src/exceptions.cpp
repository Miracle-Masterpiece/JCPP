#include <cpp/lang/exceptions.hpp>
#include <utility>

namespace jstd {


    throwable::throwable() : m_calltrace() {
        _cause[0] = 0;
    }

    throwable::throwable(const char* cause) : m_calltrace() {
        int len      = std::strlen(cause);
        int max_size = sizeof(_cause);
        int max = len >= max_size ? max_size - 1 : len;
        std::memcpy(_cause, cause, max);
        _cause[max] = 0; 
    }
    
    throwable::throwable(const throwable& t) : m_calltrace(t.m_calltrace) {
        if (&t != this)
            std::memcpy(_cause, t._cause, sizeof(_cause));
    }
    
    throwable::throwable(throwable&& t) : m_calltrace(std::move(t.m_calltrace)) {
        if (&t != this)
            std::memcpy(_cause, t._cause, sizeof(_cause));
    }
    
    throwable& throwable::operator= (const throwable& t) {
        if (&t != this) {
            m_calltrace = t.m_calltrace;
            std::memcpy(_cause, t._cause, sizeof(_cause));
        }
        return *this;
    }
    
    throwable& throwable::operator= (throwable&& t) {
        if (&t != this) {
            m_calltrace = std::move(t.m_calltrace);
            std::memcpy(_cause, t._cause, sizeof(_cause));
        }
        return *this;
    }
    
    throwable::~throwable() {

    }
    
    const char* throwable::cause() const {
        return _cause;
    }

    void throwable::print_stack_trace() const {
        m_calltrace.print();
    }
    
    const calltrace& throwable::get_calltrace() const {
        return m_calltrace;
    }
    
    void throwable::set_calltrace(calltrace&& calltrace) {
        m_calltrace = std::move(calltrace);
    }

    int throwable::to_string(char buf[], int bufsize) const {
        int len = std::strlen(_cause);
        int max = len >= bufsize ? bufsize - 1 : len;
        std::memcpy(buf, _cause, max);
        buf[max] = 0; 
        return max;
    }

#define __TEMPLATE__EXCEPT_CLASS_IMPL(clazz_name, super_clazz)                                      \
                                                                                                    \
    clazz_name::clazz_name() : super_clazz() {                                                      \
                                                                                                    \
    }                                                                                               \
                                                                                                    \
    clazz_name::clazz_name(const char* cause) : super_clazz(cause) {                                \
                                                                                                    \
    }                                                                                               \
                                                                                                    \
    clazz_name::clazz_name(const clazz_name& e) : super_clazz(e) {                                  \
                                                                                                    \
    }                                                                                               \
                                                                                                    \
    clazz_name::clazz_name(clazz_name&& e) : super_clazz(std::move(e)) {                            \
                                                                                                    \
    }                                                                                               \
                                                                                                    \
    clazz_name& clazz_name::operator=(const clazz_name& e){                                         \
        if (&e != this)                                                                             \
            super_clazz::operator=(e);                                                              \
        return *this;                                                                               \
    }                                                                                               \
                                                                                                    \
    clazz_name& clazz_name::operator=(clazz_name&& e){                                              \
        if (&e != this)                                                                             \
            super_clazz::operator=(std::move(e));                                                   \
        return *this;                                                                               \
    }                                                                                               \
                                                                                                    \
    clazz_name::~clazz_name() {                                                                     \
                                                                                                    \
    }                                                                                               \
                   
__TEMPLATE__EXCEPT_CLASS_IMPL(error,                               throwable)
__TEMPLATE__EXCEPT_CLASS_IMPL(out_of_memory_error,                 error)
__TEMPLATE__EXCEPT_CLASS_IMPL(exception,                           throwable)
__TEMPLATE__EXCEPT_CLASS_IMPL(runtime_exception,                   exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(null_pointer_exception,              runtime_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(sequrity_exception,                  runtime_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(interrupted_exception,               exception)

/**
 * #################################################################
 *                  R A N G E  E X C E P T I O N S
 * #################################################################
 */
__TEMPLATE__EXCEPT_CLASS_IMPL(index_out_of_bound_exception,        runtime_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(overflow_exception,                  runtime_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(underflow_exception,                 runtime_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(no_such_element_exception,           runtime_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(stack_overflow_error,                error)

/**
 * #################################################################
 *                  S T A T E  E X C E P T I O N S
 * #################################################################
 */
__TEMPLATE__EXCEPT_CLASS_IMPL(illegal_argument_exception,          runtime_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(illegal_state_exception,             runtime_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(unsupported_operation_exception,             exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(utf_format_exception,                runtime_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(invalid_data_format_exception,       runtime_exception)

/**
 * #################################################################
 *                  I O E X C E P T I O N S
 * #################################################################
 */
__TEMPLATE__EXCEPT_CLASS_IMPL(io_exception,                        exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(file_not_found_exception,            io_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(eof_exception,                       io_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(closed_exception,                    io_exception)

//socket exceptions
__TEMPLATE__EXCEPT_CLASS_IMPL(socket_exception,                    io_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(unknow_host_exception,               io_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(connect_exception,                   socket_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(bind_exception,                      socket_exception)
__TEMPLATE__EXCEPT_CLASS_IMPL(socket_timeout_exception,            socket_exception)


    

}