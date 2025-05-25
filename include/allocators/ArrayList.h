#ifndef ___RUNTIME_ARRAY_LIST_H_
#define ___RUNTIME_ARRAY_LIST_H_

#include <allocators/base_allocator.hpp>
#include <new>
#include <cassert>
#include <utility>
#define null nullptr

namespace tca {

template<typename T>
class array_list {
    static const int INIT_CAPACITY = 0x10;
    base_allocator*     _allocator;
    T*                  _data;
    std::size_t         _len;
    std::size_t         _capacity;
    std::size_t         _size;
    void grow();
    void cleanup();
public:
    
    array_list();
    array_list(base_allocator* allocator, std::size_t capacity = 0);
    array_list(const array_list<T>& list);
    array_list(array_list<T>&& list);
    array_list<T>& operator= (const array_list<T>& list);
    array_list<T>& operator= (array_list<T>&& list);
    ~array_list();

    template<typename _T>
    void        add(_T&&);
    
    template<typename _T>
    void        add(std::size_t idx, _T&&);
    
    T&          at(std::size_t idx);
    const T&    at(std::size_t idx) const;
    void        remove_at(std::size_t idx);
    
    std::size_t size() const;
    void        clear();
    
    T*          data() {
        return _data;
    }

    const T*    data() const {
        return _data;
    }

    array_list<T>& operator<< (const T& t){
        add(t);
        return *this;
    }
};

    template<typename T>
    array_list<T>::array_list() : _allocator(nullptr), _data(nullptr), _len(0), _capacity(0), _size(0) {}

    template<typename T>
    array_list<T>::array_list(base_allocator* allocator, std::size_t capacity) : _allocator(allocator), _data(null), _len(0), _capacity(capacity), _size(0) {
        if (_capacity > 0){
            _len    = capacity * sizeof(T);
            _data   = (T*) _allocator->allocate_align(_len, alignof(T));
            if (_data == null) throw std::bad_alloc();
        }
    }

    template<typename T>
    array_list<T>::array_list(const array_list<T>& list) : _allocator(nullptr), _data(null), _len(0), _capacity(0), _size(0) {
        if (list._size > 0){
            _allocator  = list._allocator;
            _capacity   = list._size;
            _size       = list._size;
            _len        = _size * sizeof(T);
            _data       = _allocator->allocate_align(_len, alignof(T));
            if (_data == null) throw std::bad_alloc();
            for (std::size_t i = 0; i < _size; ++i){
                new (_data + i) T(list._data[i]);
            }
        }
    }

    template<typename T>
    array_list<T>::array_list(array_list<T>&& list) : _allocator(list._allocator), _data(list._data), _len(list._len), _capacity(list._capacity), _size(list._size) {
        list._allocator = nullptr;
        list._data      = nullptr;
        list._len       = 0;
        list._capacity  = 0;
        list._size      = 0;
    }

    template<typename T>
    array_list<T>& array_list<T>::operator=(const array_list<T>& list){
        if (&list != this){
            if (list._capacity < list._size){
                cleanup();
                _allocator  = list._allocator;
                _capacity   = list._size;
                _size       = list._size;
                _len        = _size * sizeof(T);
                _data   = (T*) _allocator->allocate_align(_len, alignof(T));
                if (_data == null) throw std::bad_alloc();
            }
            for (std::size_t i = 0; i < _size; ++i){
                new (_data + i) T(list._data[i]);
            }
        }
        return *this;
    }

    template<typename T>
    array_list<T>& array_list<T>::operator=(array_list<T>&& list){
        if (&list != this){
            cleanup();
            _data = list._data;
            list._data = null;

            _len = list._len;
            list._len = 0;

            _capacity = list._capacity;
            list._capacity = 0;

            _size = list._size;
            list._size = 0;

            _allocator = list._allocator;
            list._allocator = nullptr;
        }
        return *this;
    }

    template<typename T>
    void array_list<T>::cleanup() {
        if (_data != null){
            clear();
            _allocator->deallocate(_data, _len);
            _data       = null;
            _len        = 0;
            _capacity   = 0;
            _size       = 0;
        }
    }

    template<typename T>
    array_list<T>::~array_list() {
        cleanup();
    }

    template<typename T>
    void array_list<T>::grow() {
        if (_capacity == 0) _capacity = INIT_CAPACITY;
        
        std::size_t oldlen  = _len;
        std::size_t newCapacity     = _capacity << 1;
        std::size_t newlen  = newCapacity * sizeof(T);
        
        T* olddata = _data;
        
        T* newdata = (T*) _allocator->allocate_align(newlen, alignof(T));
        if (newdata == null) 
            throw std::bad_alloc();

        for (std::size_t i = 0; i < _size; ++i){
            new (newdata + i) T(std::move(olddata[i]));
        }

        _allocator->deallocate(olddata, oldlen);
        _data       = newdata;
        _len        = newlen;
        _capacity   = newCapacity;
    }

    template<typename T>
    template<typename _T>
    void array_list<T>::add(_T&& t) {
        if (_size + 1 > _capacity)
            grow();
        new (_data + (_size++)) T(std::forward<_T>(t));
    }

    template<typename T>
    template<typename _T>
    void array_list<T>::add(std::size_t idx, _T&& t) {
        assert(idx <= _size);
        if (idx == _size){
            add(std::forward<_T>(t));
            return;
        }
        
        if (_size >= _capacity)
            grow();

        for (std::size_t i = _size; i > idx; --i){
            new (_data + i) T(std::move(_data[i - 1]));
        }

        new (_data + idx) T(std::forward<_T>(t));
        ++_size;
    }

    template<typename T>
    T& array_list<T>::at(std::size_t idx) {
        assert(idx < _size);
        return _data[idx];
    }

    template<typename T>
    const T& array_list<T>::at(std::size_t idx) const {
        assert(idx < _size);
        return _data[idx];
    }

    template<typename T>
    std::size_t array_list<T>::size() const {
        return _size;
    }
    
    template<typename T>
    void array_list<T>::clear() {
        if (_size > 0) {
            for (std::size_t i = _size - 1; ; --i){
                _data[i].~T();
                if (i == 0)
                    break;
            }
        } 
        _size = 0;
    }

    template<typename T>
    void array_list<T>::remove_at(std::size_t idx) {
        assert(idx < _size);
        _data[idx].~T();
        for (std::size_t i = idx; i < _size - 1; ++i){
            new(_data + i) T(std::move(_data[i + 1]));
        }
        --_size;
    }
}


#endif//___RUNTIME_ARRAY_LIST_H_