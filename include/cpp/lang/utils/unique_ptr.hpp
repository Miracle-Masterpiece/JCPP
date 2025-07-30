#ifndef JSTD_CPP_LANG_UTILS_UNIQUE_PTR_H
#define JSTD_CPP_LANG_UTILS_UNIQUE_PTR_H

#include <allocators/base_allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/arrays.hpp>
#include <utility>
#include <cstdint>

namespace jstd {

template<typename E>
class unique_ptr {
    unique_ptr(const unique_ptr<E>&) = delete;
    unique_ptr& operator= (const unique_ptr<E>&) = delete;

    tca::base_allocator*    m_allocator;
    E*                      m_element;
    
    inline void check_access() const;
    void cleanup();
public:
    unique_ptr();
    unique_ptr(tca::base_allocator* allocator);
    unique_ptr(tca::base_allocator* allocator, E* data);
    template<typename _E>
    unique_ptr(tca::base_allocator* allocator, _E&&);
    unique_ptr(unique_ptr<E>&&);
    unique_ptr<E>& operator= (unique_ptr<E>&&);
    ~unique_ptr();
    
    E* release();
    E& operator*() const;
    E* operator->() const;
    E* operator() () const;
    E* raw_ptr() const;
    E& operator[] (std::size_t idx) const;
    bool is_null() const;

};
    template<typename E>
    unique_ptr<E> take(tca::base_allocator* allocator, E* ptr) {
        return unique_ptr<E>(allocator, ptr);
    }
    
    template<typename E>
    unique_ptr<E>::unique_ptr() : m_allocator(nullptr), m_element(nullptr) {

    }

    template<typename E>
    unique_ptr<E>::unique_ptr(tca::base_allocator* allocator) : unique_ptr<E>(allocator, E()) {
        
    }
    
    template<typename E>
    unique_ptr<E>::unique_ptr(tca::base_allocator* allocator, E* data) : m_allocator(allocator), m_element(data) {
        
    }

    template<typename T>
    unique_ptr<T> make_unique(T&& object, tca::base_allocator* allocator = tca::get_scoped_or_default()) {
        return unique_ptr<T>(allocator, std::forward<T>(object));
    }

    template<typename E>
    template<typename _E>
    unique_ptr<E>::unique_ptr(tca::base_allocator* allocator, _E&& e) {
        void* raw_data = allocator->allocate_align(sizeof(E), alignof(E));
        if (raw_data == nullptr)
            throw_except<out_of_memory_error>("Out of memory!");
        m_element   = new(raw_data) E(std::forward<_E>(e));
        m_allocator = allocator;
    }
    
    template<typename E>
    unique_ptr<E>::unique_ptr(unique_ptr<E>&& p) : m_allocator(p.m_allocator), m_element(p.m_element) {
        p.m_allocator   = nullptr;
        p.m_element     = nullptr;
    }
    
    template<typename E>
    unique_ptr<E>& unique_ptr<E>::operator= (unique_ptr<E>&& p) {
        if (&p != this) {
            cleanup();
            m_allocator = p.m_allocator;
            m_element   = p.m_element;
            
            p.m_allocator   = nullptr;
            p.m_element     = nullptr;
        }
        return *this;
    }

    template<typename E>
    void unique_ptr<E>::cleanup() {
        if (m_allocator != nullptr && m_element != nullptr) {
            {//delete
                m_element->~E();
                m_allocator->deallocate(m_element, sizeof(E));
            }
            m_element   = nullptr;
            m_allocator = nullptr;
        }
    }

    template<typename E>
    unique_ptr<E>::~unique_ptr() {
        cleanup();
    }

    template<typename E>
    void unique_ptr<E>::check_access() const {
        #ifndef NDEBUG
        if (m_element == nullptr)
            throw_except<null_pointer_exception>();
        #endif//NDEBUG
    }
    
    template<typename E>
    E& unique_ptr<E>::operator*() const {
        check_access();
        return *m_element;
    }

    template<typename E>
    E* unique_ptr<E>::operator->() const {
        check_access();
        return m_element;
    }

    template<typename E>
    E* unique_ptr<E>::release() {
        check_access();
        E* data = m_element;
        m_allocator = nullptr;
        m_element   = nullptr;
        return data;
    }

    template<typename E>
    E* unique_ptr<E>::operator() ()  const {
        check_access();
        return m_element;
    }

    template<typename E>
    bool unique_ptr<E>::is_null() const {
        return m_element == nullptr;
    }

    template<typename E>
    E* unique_ptr<E>::raw_ptr() const {
        return reinterpret_cast<E*>(m_element);
    }

    template<typename E>
    E& unique_ptr<E>::operator[] (std::size_t idx) const {
        check_access();
        return m_element[idx];
    }
    
template<typename E>
class unique_ptr<E[]> {
    
    tca::base_allocator*    m_allocator;
    E*                      m_element;
    int64_t                 m_length;

    inline void check_access() const;

public:
    unique_ptr();
    unique_ptr(tca::base_allocator* allocator, int64_t length);
    ~unique_ptr();
    bool is_null() const;
    E& operator*() const;
    E* operator->() const;
    E& operator[] (std::size_t idx) const;
    E* raw_ptr() const;
};

    template<typename E>
    unique_ptr<E[]>::unique_ptr() : m_allocator(nullptr), m_element(nullptr), m_length(0) {
        
    }

    template<typename E>
    unique_ptr<E[]>::unique_ptr(tca::base_allocator* allocator, int64_t length) : unique_ptr<E[]>() {
        E* data = (E*) allocator->allocate_align(sizeof(E) * length, alignof(E));
        if (data == nullptr)
            throw_except<out_of_memory_error>("Out of memory!");        
        placement_new(data, length);
        m_element   = data;
        m_allocator = allocator;
        m_length    = length;
    }

    template<typename E>
    unique_ptr<E[]>::~unique_ptr() {
        if (m_allocator != nullptr && m_element != nullptr) {
            placement_destroy(m_element, m_length);
            m_allocator->deallocate(m_element, sizeof(E) * m_length);
            m_element   = nullptr;
            m_allocator = nullptr;
        }
    }

    template<typename E>
    void unique_ptr<E[]>::check_access() const {
        #ifndef NDEBUG
        if (m_element == nullptr)
            throw_except<null_pointer_exception>();
        #endif//NDEBUG
    }

    template<typename E>
    E& unique_ptr<E[]>::operator[] (std::size_t idx) const {
        check_access();
        if (idx > m_length)
            throw_except<index_out_of_bound_exception>("Index %zu out of bound for length %zu", idx, (std::size_t) m_length);
        return m_element[idx];
    }

    template<typename E>
    bool unique_ptr<E[]>::is_null() const {
        return m_element == nullptr;
    }

    template<typename E>
    E* unique_ptr<E[]>::raw_ptr() const {
        check_access();
        return m_element;
    }
};

#endif//JSTD_CPP_LANG_UTILS_UNIQUE_PTR_H
