#ifndef _ALLOCATORS_STRING_H
#define _ALLOCATORS_STRING_H

#include <allocators/base_allocator.hpp>
#include <cpp/lang/system.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/utils.hpp>
#include <cpp/lang/utils/objects.hpp>

#include <cassert>
#include <cstring>
#include <utility>
#include <exception>
#include <algorithm>
#include <iostream>

namespace jstd {

template<typename CHAR_TYPE>
class tstring;

/**
 * Этот класс представляет динамически выделенную строку, поддерживающую различные операции, такие как
 * добавление, замена, подсчёт вхождений и извлечение подстрок. Он предоставляет гибкий
 * интерфейс для работы со строками с учётом типа символов и стратегий выделения памяти.
 * Класс параметризован, что позволяет использовать разные типы символов, обеспечивая гибкость в использовании
 * памяти и манипуляциях с ней, а также поддерживает порядок байтов для систем с различным порядком байтов.
 * 
 * @tparam CHAR_TYPE Тип символа, используемого для строки (например, char, wchar_t и т.д.).
 */
template<typename CHAR_TYPE>
class tstring {
protected:
    
    tca::base_allocator* _allocator; // Аллокатор памяти для хранения строки.
    CHAR_TYPE* _data;                // Указатель на данные строки.
    int _capacity;                   // Вместимость выделенной памяти.
    int _size;                       // Текущий размер (длина) строки.
    char _order;                     // Порядок байтов строки (младший порядок или старший порядок).
    //TODO: даже не спрашивай какого чёрта функция просто не перенесена в секцию public
public:
     /**
     * Вспомогательная функция для вычисления длины строки.
     * @param str Строка, длину которой нужно вычислить.
     * @return Длина строки.
     */
    static int strlen(const CHAR_TYPE* str) {
        int len = 0;
        while (*(str + len)) 
            ++len;
        return len;
    }

protected:
    /**
     * Нормализует параметр длины. Если длина отрицательная, используется фактическая длина строки.
     * @param s Строка для измерения.
     * @param len Длина для нормализации.
     * @return Нормализованная длина.
     */
    static int normalize_length(const CHAR_TYPE* s, int len) {
        if (len < 0)
            return tstring<CHAR_TYPE>::strlen(s);
        return len;
    }

    /**
     * Удаляет ресурсы текущей строки, освобождая память.
     */
    void dispose();        
    
    /**
     * Изменяет размер строки до заданного значения.
     * 
     * @remark Учитывает нулевой символ.
     * 
     * @param sz Новый размер строки.
     */
    void resize_for(int sz);
    
    /**
     * Изменяет размер строки, выделяя новую память.
     * 
     * @remark Не учитывает нулевой символ.
     * 
     * @param new_size Новый размер для строки.
     */
    void resize(int new_size);
    
    /**
     * Проверяет, что аллокатор валиден (не равен нулю).
     * @throws illegal_state_exception Если аллокатор равен null.
     */
    void check_allocator() const {
        if (_allocator == nullptr)
            throw_except<illegal_state_exception>("Allocator is null!");
    }
    
    /**
     * Конструирует строку из переданной строки, выделяя память и копируя данные.
     * @param str Исходная строка.
     * @param allocator Аллокатор памяти.
     * @param in Порядок байтов исходной строки.
     * @param out Порядок байтов для созданной строки.
     */
    void construct_string(const CHAR_TYPE* str, tca::base_allocator* allocator, byte_order in, byte_order out) {
        int len = tstring<CHAR_TYPE>::strlen(str);
        CHAR_TYPE* data = (CHAR_TYPE*) allocator->allocate_align(sizeof(CHAR_TYPE) * (len + 1), alignof(CHAR_TYPE));
        if (data == nullptr)
            throw_except<out_of_memory_error>("Out of memory!");
        _allocator  = allocator;
        _data       = data;
        _capacity   = sizeof(CHAR_TYPE) * (len + 1);
        _size       = len;
        _order      = out;
        if (sizeof(CHAR_TYPE) == sizeof(char) || in == out) {
            std::memcpy(_data, str, sizeof(CHAR_TYPE) * len);
        } else {
            utils::copy_swap_memory(_data, str, sizeof(CHAR_TYPE), len);
        }
        _data[_size] = 0;
    }

    /**
     * Устанавливает символ в заданном индексе, учитывая порядок байтов.
     * @param idx Индекс для установки символа.
     * @param ch Символ для установки.
     * @param ch_order Порядок байтов символа.
     */
    void set_char(int idx, CHAR_TYPE ch, byte_order ch_order) {
        check_index(idx, _capacity);
        if (sizeof(CHAR_TYPE) == sizeof(char) || ch_order == _order) {
            _data[idx] = ch;
        } else {
            _data[idx] = utils::swap_bytes<CHAR_TYPE>(ch);
        }
    }

    /**
     * Получает символ в заданном индексе, учитывая порядок байтов.
     * @param idx Индекс для получения символа.
     * @param ret_order Порядок байтов для возвращаемого символа.
     * @return Символ в указанном индексе.
     */
    CHAR_TYPE get_char(int idx, byte_order ret_order) const {
        check_index(idx, _capacity);
        if (sizeof(CHAR_TYPE) == sizeof(char) || ret_order == _order) {
            return _data[idx];
        } else {
            return utils::swap_bytes<CHAR_TYPE>(_data[idx]);
        }
    }

public:
    /**
     * Конструктор по умолчанию. Инициализирует пустую строку.
     */
    tstring();
    
    /**
     * Конструктор, инициализирующий строку с заданным аллокатором.
     * @param allocator Аллокатор для управления памятью.
     */
    tstring(tca::base_allocator* allocator);
   
    /**
     * Конструктор, инициализирующий строку с заданным аллокатором и исходной строкой.
     * @param allocator Аллокатор для управления памятью.
     * @param str Строка для инициализации.
     * @param in Порядок байтов исходной строки.
     * @param out Порядок байтов для созданной строки.
     */    
    tstring(tca::base_allocator* allocator, const CHAR_TYPE* str, byte_order in = system::native_byte_order(), byte_order out = system::native_byte_order());
    
    /**
     * Конструктор копирования.
     * @param str Строка для копирования.
     */
    tstring(const tstring<CHAR_TYPE>& str);
    
    /**
     * Конструктор перемещения.
     * @param str Строка для перемещения.
     */
    tstring(tstring<CHAR_TYPE>&& str);
    
    /**
     * Оператор копирования.
     * @param str Строка для копирования.
     * @return Ссылка на текущую строку.
     */
    tstring& operator= (const tstring<CHAR_TYPE>& str);

    /**
     * Оператор перемещения.
     * @param str Строка для перемещения.
     * @return Ссылка на текущую строку.
     */
    tstring& operator= (tstring<CHAR_TYPE>&& str);
    
    /**
     * Деструктор, который освобождает выделенную память.
     */
    ~tstring();
    
    /**
     * Добавляет строку к текущей строке.
     * @param str Строка для добавления.
     * @param slen Длина добавляемой строки. Если -1, используется вся строка.
     * @param str_order Порядок байтов добавляемой строки.
     * @return Ссылка на текущую строку.
     */
    tstring<CHAR_TYPE>& append(const CHAR_TYPE* str, int slen = -1, byte_order str_order = system::native_byte_order());
    
    /**
     * Добавляет строку к текущей строке с заданного индекса.
     * @param idx Индекс, с которого нужно добавить.
     * @param str Строка для добавления.
     * @param slen Длина добавляемой строки. Если -1, используется вся строка.
     * @param str_order Порядок байтов добавляемой строки.
     * @return Ссылка на текущую строку.
     */
    tstring<CHAR_TYPE>& append(int idx, const CHAR_TYPE* str, int slen = -1, byte_order str_order = system::native_byte_order());
    
    /**
     * Добавляет строку к текущей строке с заданного индекса.
     * @param idx Индекс, с которого нужно добавить.
     * @param str Строка для добавления.
     * @return Ссылка на текущую строку.
     */
    tstring<CHAR_TYPE>& append(int idx, const tstring<CHAR_TYPE>& str);

    /**
     * Добавляет строку к текущей строке.
     * @param str Строка для добавления.
     * @return Ссылка на текущую строку.
     */
    tstring<CHAR_TYPE>& append(const tstring<CHAR_TYPE>& str);

    /**
     * Заменяет вхождения подстроки на другую строку.
     * @param regex Подстрока для замены.
     * @param replacement Строка для замены.
     * @param regex_length Длина заменяймой подстроки.
     * @param replacement_length Длина заменяющей строки.
     * @param regex_order Порядок байтов заменяймой подстроки.
     * @param replacement_order Порядок байтов заменяющей.
     * @return Ссылка на текущую строку.
     */
    tstring<CHAR_TYPE>& replace(
        const CHAR_TYPE* regex, 
        const CHAR_TYPE* replacement, 
        int regex_length             = -1, 
        int replacement_length       = -1, 
        byte_order regex_order       = system::native_byte_order(), 
        byte_order replacement_order = system::native_byte_order());

    tstring<CHAR_TYPE>& replace(
        int idx,
        const CHAR_TYPE* regex, 
        const CHAR_TYPE* replacement, 
        int regex_length             = -1, 
        int replacement_length       = -1, 
        byte_order regex_order       = system::native_byte_order(), 
        byte_order replacement_order = system::native_byte_order());

    /**
     * Подсчитывает количество вхождений подстроки в текущую строку.
     * @param str Подстрока для подсчёта.
     * @param len Длина подстроки. Если -1, используется вся строка.
     * @param str_order Порядок байтов подстроки.
     * @return Количество вхождений подстроки.
     */
    int count_contains(const CHAR_TYPE* str, int len = -1, byte_order str_order = system::native_byte_order()) const;
    
    /**
     * Проверяет, содержит ли текущая строка подстроку.
     * @param str Подстрока для проверки.
     * @param len Длина подстроки. Если -1, используется вся строка.
     * @param str_order Порядок байтов подстроки.
     * @return true, если подстрока найдена, false в противном случае.
     */
    bool contains(const CHAR_TYPE* str, int len = -1, byte_order = system::native_byte_order()) const;

    /**
     * Находит индекс первого вхождения подстроки.
     * @param str Подстрока для поиска.
     * @param len Длина подстроки. Если -1, используется вся строка.
     * @param str_order Порядок байтов подстроки.
     * @return Индекс первого вхождения подстроки или -1, если не найдено.
     */
    int index_of(const CHAR_TYPE* str, int len = -1, byte_order = system::native_byte_order()) const;
    
    /**
     * Находит индекс последнего вхождения подстроки.
     * @param str Подстрока для поиска.
     * @param len Длина подстроки. Если -1, используется вся строка.
     * @param str_order Порядок байтов подстроки.
     * @return Индекс последнего вхождения подстроки или -1, если не найдено.
     */
    int last_index_of(const CHAR_TYPE* str, int len = -1, byte_order = system::native_byte_order()) const;

    /**
     * Проверяет, начинается ли строка с указанной подстроки.
     * @param str Подстрока для проверки.
     * @param len Длина подстроки. Если -1, используется вся строка.
     * @param str_order Порядок байтов подстроки.
     * @return true, если строка начинается с подстроки, false в противном случае.
     */
    bool starts_with(const CHAR_TYPE* str, int len = -1, byte_order = system::native_byte_order()) const;
    
    /**
     * Проверяет, заканчивается ли строка на указанную подстроку.
     * @param str Подстрока для проверки.
     * @param len Длина подстроки. Если -1, используется вся строка.
     * @param str_order Порядок байтов подстроки.
     * @return true, если строка заканчивается на подстроку, false в противном случае.
     */
    bool ends_with(const CHAR_TYPE* str, int len = -1, byte_order = system::native_byte_order()) const;

    /**
     * Возвращает необработанное представление строки как C-строки.
     * @return Указатель на C-строку.
     */
    const CHAR_TYPE* c_string() const {
        return _data;
    }

    /**
     * Возвращает порядок байтов строки.
     * @return Порядок байтов строки.
     */
    byte_order order() const {
        return (byte_order)  _order;
    }
    
    /**
     * Устанавливает порядок байтов строки.
     * Данная функция не переконвертирует уже находящиеся символы в строке из одного порядка в другой. 
     * Но новые символы будут кодироваться в соотвествии с текущим порядком байт.
     */
    void order(byte_order order) {
        _order = (char) order;
    }

    /**
     * Возвращает вместимость (выделенный размер) строки. (НЕ В БАЙТАХ!!!)
     * @return Вместимость строки.
     */
    int capacity() const {
        return _capacity;
    }

    /**
     * Возвращает текущую длину строки. (НЕ В БАЙТАХ!!!)
     * @return Длина строки.
     */
    int length() const {
        return _size;
    }

    /**
     * Резервирует память для строки, изменяя её вместимость.
     * @param size Размер для резервирования.
     * @return Ссылка на текущую строку.
     */
    tstring<CHAR_TYPE>& reserve(int size);
    
    /**
     * Обрезает ведущие и хвостовые пробелы в строке.
     * @return Ссылка на текущую строку.
     */
    tstring<CHAR_TYPE>& trim();
    
    /**
     * Обрезает строку до её текущего размера (освобождает неиспользуемую память).
     * @return Ссылка на текущую строку.
     */
    tstring<CHAR_TYPE>& trim_to_size();
    
    /**
     * Зануляет память между this->_size и this->_capacity
     * @return Ссылка на текущую строку.
     */
    tstring<CHAR_TYPE>& wipe_extra();
    
    /**
     * Очищает строку, устанавливая её размер в ноль.
     * @remark Выделенная память не освобождается!
     * @return Ссылка на текущую строку.
     */
    tstring<CHAR_TYPE>& clear();
    
     /**
     * Безопасно очищает строку, зануляя её память.
     * @remark Выделенная память не освобождается!
     * @return Ссылка на текущую строку.
     */
    tstring<CHAR_TYPE>& secure_clear();

    /**
     * Удаляет часть строки, начиная с заданного индекса.
     * @param start Начальный индекс для удаления.
     * @param end Конечный индекс для удаления.
     * 
     * @remark подстрока удаляется до символа end, не включительно!
     * 
     * @return Ссылка на текущую строку.
     */
    tstring<CHAR_TYPE>& remove(int start, int end);
    
    /**
     * Клонирует строку. И аллоцирует её содержимое в передаваемом аллокаторе.
     * Если в аргумент аллокатора передаётся nullptr, то метод будет использовать внутренний аллокатор текущей строки.
     * 
     * @param allocator
     *       Аллокатор который будет отвечать за распределение памяти новой строки.
     * @return
     *      Копия текущей строки.
     */
    tstring<CHAR_TYPE> clone(tca::base_allocator* allocator = nullptr) const;
    
    /**
     * Возвращает содержимое подстроки текущей строки.
     * @param start     Индекс начала
     * @param end       Индекс конца(не включительно!)
     * @param allocator Аллокатор, в который будет отвечать за распределение памяти для новой строки.
     *                  Если nullptr, то будет взят аллокатор текущей строки.
     * @return Новая строка, содержащая подстроку.
     */
    tstring<CHAR_TYPE> substr(int start, int end, tca::base_allocator* allocator = nullptr);

    /**
     * Получить символ по заданному индексу.
     * @param idx Индекс для получения символа.
     * @return Символ по заданному индексу.
     */
    CHAR_TYPE char_at(int idx) const {
        return get_char(idx, system::native_byte_order());
    }

    /**
     * Вычисляет хеш-код строки.
     */
    uint64_t hashcode() const;
    
    /**
     * Проверяет, идентичны ли строки.
     * 
     * @return  
     *      true - если строки равны, иначе false.
     */
    bool equals(const tstring<CHAR_TYPE>& str) const;

    /**
     * Проверяет, идентичны ли строки.
     * 
     * @return  
     *      true - если строки равны, иначе false.
     */
    bool equals(const CHAR_TYPE* str, int slen = -1, byte_order str_order = system::native_byte_order()) const;

    /**
     * ==============================================================
     *                  D E B U G  F U N CS
     * ==============================================================
     */
    void print() const;
};

    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>::tstring() : _allocator(nullptr), _data(nullptr), _capacity(0), _size(0), _order(-1) {
    }
    
    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>::tstring(tca::base_allocator* allocator) : _allocator(allocator), _data(nullptr), _capacity(0), _size(0), _order(-1) {
    }
    
    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>::tstring(tca::base_allocator* allocator, const CHAR_TYPE* str, byte_order in, byte_order out) : tstring<CHAR_TYPE>() {
        if (str != nullptr) {
            construct_string(str, allocator, in, out);
        } else {
            _order      = out;
            _allocator  = allocator;
        }
    }

    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>::tstring(const tstring<CHAR_TYPE>& str) : tstring() {
        if (str._allocator != nullptr)
            construct_string(str._data, str._allocator, (byte_order) str._order, (byte_order) str._order);
    }
    
    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>::tstring(tstring<CHAR_TYPE>&& str) : 
    _allocator(str._allocator), 
    _data(str._data), 
    _capacity(str._capacity), 
    _size(str._size), 
    _order(str._order) {
        str._allocator  = nullptr;
        str._data       = nullptr;
        str._capacity   = 0;
        str._size       = 0;
    }
    
    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::operator= (const tstring<CHAR_TYPE>& str) {
        if (&str != this) {
            tstring<CHAR_TYPE> tmp(str);
            this->operator=(std::move(tmp));
        }
        return *this;
    }
    
    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::operator= (tstring<CHAR_TYPE>&& str) {
        if (&str != this) {
            dispose();
            _allocator  = str._allocator;
            _data       = str._data;
            _capacity   = str._capacity;
            _size       = str._size;
            _order      = str._order;
            str._allocator  = nullptr;
            str._data       = nullptr;
            str._capacity   = 0;
            str._size       = 0;
        }
        return *this;
    }
    
    template<typename CHAR_TYPE>
    void tstring<CHAR_TYPE>::dispose() {
        if (_allocator != nullptr && _data != nullptr) {
            _allocator->deallocate(_data, sizeof(CHAR_TYPE) * _capacity);
            _data       = nullptr;
            _capacity   = 0;
            _size       = 0;
        }
    }

    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>::~tstring() {
        dispose();
    }

    template<typename CHAR_TYPE>
    void tstring<CHAR_TYPE>::resize(int sz) {
        check_allocator();
        CHAR_TYPE* data = (CHAR_TYPE*) _allocator->allocate_align(sizeof(CHAR_TYPE) * sz, alignof(CHAR_TYPE));
        if (data == nullptr)
            throw_except<out_of_memory_error>("Out of memory!");
        //std::memset(data, '\0', sizeof(CHAR_TYPE) * sz);
        std::memcpy(data, _data, sizeof(CHAR_TYPE) * _size);
        _allocator->deallocate(_data, sizeof(CHAR_TYPE) * _capacity);
        _data       = data;
        _capacity   = sz;
    }

    template<typename CHAR_TYPE>
    void tstring<CHAR_TYPE>::resize_for(int new_size) {
        if (new_size == 1) {
            if (_capacity < new_size) 
                resize(new_size);
        } else {
            int newCapacity = std::max(_capacity, 2);
            while (new_size > newCapacity) {
                newCapacity *= 1.5;
            }
            resize(newCapacity);
        }
    }

    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::reserve(int size) {
        resize(size + 1);
        return *this;
    }

    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::append(const CHAR_TYPE* str, int slen, byte_order str_order) {
        return append(_size, str, slen, str_order);
    }
    
    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::append(int idx, const CHAR_TYPE* str, int slen, byte_order str_order) {
        check_index(idx, _size + 1);
        int len = normalize_length(str, slen);
        if (_size + len + 1 > _capacity)
            resize_for(_size + len + 1);
        if (idx != _size) {
            CHAR_TYPE* dst = _data + idx + len;
            CHAR_TYPE* src = _data + idx;
            std::memmove(dst, src, sizeof(CHAR_TYPE) * (_size - idx));
        }
        for (int i = 0; i < len; ++i) {
            set_char(idx + i, str[i], str_order);
            ++_size;
        }
        _data[_size] = 0;
        return *this;
    }

    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::append(int idx, const tstring<CHAR_TYPE>& str) {
        return append(idx, str.c_string(), str.length(), (byte_order) str._order);
    }
    
    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::append(const tstring<CHAR_TYPE>& str) {
        return append(str._data, str._size, (byte_order) str._order);
    }

    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::replace(
        const CHAR_TYPE* regex, 
        const CHAR_TYPE* replacement, 
        int regex_length, 
        int replacement_length, 
        byte_order regex_order, 
        byte_order replacement_order) 
    {
        regex_length        = normalize_length(regex, regex_length);
        replacement_length  = normalize_length(replacement, replacement_length);    
        int count_equal     = count_contains(regex, regex_length, regex_order);

        if (regex_length > _size || count_equal == 0)
            return *this;
        
        int size = _size - (regex_length * count_equal) + (replacement_length * count_equal); 
        if (size + 1 > _capacity)
            resize_for(size + 1);

        for (int i = 0; i < _size;) {
            if (i + regex_length > _size)
                break;
            bool contains = true;
            for (int j = 0; j < regex_length; ++j) {
                const CHAR_TYPE ch1 = get_char(i + j, regex_order);
                const CHAR_TYPE ch2 = regex[j];
                if (ch1 != ch2){
                    contains = false;
                    break;
                }
            }
            if (contains) {
                replace(i, regex, replacement, regex_length, replacement_length, regex_order, replacement_order);
                i += replacement_length;
            } else {
                ++i;
            }
        }

        size = _size;
        _data[size] = 0;

        return *this;
    }

    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::replace(
        int idx,
        const CHAR_TYPE* regex, 
        const CHAR_TYPE* replacement, 
        int regex_length, 
        int replacement_length, 
        byte_order regex_order, 
        byte_order replacement_order) 
    {
        check_index(idx, _size);
        regex_length        = normalize_length(regex, regex_length);
        replacement_length  = normalize_length(replacement, replacement_length);
        int size = _size - regex_length + replacement_length;
        
        //[......................|...|..|.....]

        CHAR_TYPE* in   = _data + idx + regex_length;
        CHAR_TYPE* out  = _data + idx + replacement_length;
        int off         = _size - (idx + regex_length);
        std::memmove(out, in, sizeof(CHAR_TYPE) * off);

        for (int i = 0; i < replacement_length; ++i)
            set_char(idx + i, replacement[i], replacement_order);

        _size = size;
        _data[_size] = 0;
        return *this;
    }

    template<typename CHAR_TYPE>
    int tstring<CHAR_TYPE>::count_contains(const CHAR_TYPE* str, int len, byte_order str_order) const {
        len = normalize_length(str, len);
        if (len > _size)
            return 0;

        int count_contains = 0;
        for (int i = 0; i < _size; ) {
            if (i + len > _size)
                return count_contains;
            bool contains = true;
            for (int j = 0; j < len; ++j) {
                const CHAR_TYPE ch1 = get_char(i + j, str_order);
                const CHAR_TYPE ch2 = str[j];
                if (ch1 != ch2) {
                    contains = false;
                    break;
                }
            }
            if (contains) {
                ++count_contains;
                i += len;
            } else {
                ++i;
            }
        }

        return count_contains;
    }

    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::trim() {
        if (_size == 0)
            return *this;
        
        {//начало
            int end = 0;
            for (int i = 0; i < _size; ++i) {
                if (get_char(i, system::native_byte_order()) <= ' ') {
                    ++end;
                } else {
                    break;
                }
            }
            if (end > 0)
                remove(0, end);
        }

        {//конец
            int start = _size;
            for (int i = _size - 1; i >= 0; --i) {
                if (get_char(i, system::native_byte_order()) <= ' ') {
                    --start;
                } else {
                    break;
                }
            }
            if (start != _size)
                remove(start, _size);
        }

        return *this;
    }
    
    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::trim_to_size() {
        if (_size != 0) {
            resize(_size + 1);
            _data[_size] = 0;
        }
        else {
            dispose();
        }
        return *this;
    }
    
    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::wipe_extra() {
        if (_size != 0)
            std::memset(_data + _size, 0, sizeof(CHAR_TYPE) * (_capacity - _size));
        return *this;
    }
    
    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::clear() {
        _size = 0;
        if (_data != nullptr && _capacity > 0)
            _data[0] = 0;
        return *this;
    }
    
    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::secure_clear() {
        if (_size != 0) {
            clear();
            std::memset(_data, 0, sizeof(CHAR_TYPE) * _capacity);
        }
        return *this;
    }

    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE>& tstring<CHAR_TYPE>::remove(int start, int end) {
        check_index(start,  _size + 1);
        check_index(end,    _size + 1);
        if (start > end)
            throw_except<illegal_argument_exception>("start = %i > end = %i", start, end);
        int len = end - start;
        std::memmove(_data + start, _data + end, sizeof(CHAR_TYPE) * (_size - end));
        _size -= len;
        _data[_size] = 0;
        return *this;
    }

    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE> tstring<CHAR_TYPE>::clone(tca::base_allocator* allocator) const {
        if (_allocator == nullptr && allocator == nullptr) 
            return tstring<CHAR_TYPE>();
        else if (allocator != nullptr) 
            return tstring<CHAR_TYPE>(allocator, _data);
        return tstring<CHAR_TYPE>(_allocator, _data);
    }

    template<typename CHAR_TYPE>
    bool tstring<CHAR_TYPE>::contains(const CHAR_TYPE* str, int in_len, byte_order str_order) const {
        return index_of(str, in_len, str_order) != -1;
    }

    template<typename CHAR_TYPE>
    int tstring<CHAR_TYPE>::index_of(const CHAR_TYPE* str, int in_len, byte_order str_order) const {
        in_len = normalize_length(str, in_len);
        if (in_len > _size)
            return -1;
        for (int i = 0; i < _size; ++i) {
            if (i + in_len > _size)
                return -1;
            bool contains = true;    
            for (int j = 0; j < in_len; ++j) {
                if (get_char(i + j, str_order) != str[j]) {
                    contains = false;
                    break;
                }
            }   
            if (contains)
                return i;
        }
        return -1;
    }
    
    template<typename CHAR_TYPE>
    int tstring<CHAR_TYPE>::last_index_of(const CHAR_TYPE* str, int in_len, byte_order str_order) const {
        in_len = normalize_length(str, in_len);
        if (in_len > _size)
            return -1;
        
        for (int i = _size - 1; i >= 0; --i) {
            if (i - in_len < 0)
                return -1;
            bool contains = true;
            for (int j = in_len - 1; j >= 0; --j) {
                if (get_char(i + j, str_order) != str[j]) {
                    contains = false;
                    break;
                }
                if (contains)
                    return i;
            }
        }
            
        return -1;
    }

    template<typename CHAR_TYPE>
    bool tstring<CHAR_TYPE>::starts_with(const CHAR_TYPE* str, int in_len, byte_order str_order) const {
        in_len = normalize_length(str, in_len);
        if (in_len > _size)
            return false;
        for (int i = 0; i < in_len; ++i)
            if (get_char(i, str_order) != str[i])
                return false;
        return true;
    }

    template<typename CHAR_TYPE>
    bool tstring<CHAR_TYPE>::ends_with(const CHAR_TYPE* str, int in_len, byte_order str_order) const {
        in_len = normalize_length(str, in_len);
        if (in_len > _size)
            return false;
        for (int i = _size - 1, off = in_len - 1; off >= 0; --i, --off)
            if (get_char(i, str_order) != str[off])
                return false;
        return true;
    }

    template<typename CHAR_TYPE>
    tstring<CHAR_TYPE> tstring<CHAR_TYPE>::substr(int start, int end, tca::base_allocator* allocator) {
        check_index(start,  _size + 1);
        check_index(end,    _size + 1);
        if (start > end)
            throw_except<illegal_argument_exception>("start = %i > end = %i", start, end);
        int len = end - start;
        tstring<CHAR_TYPE> result(allocator != nullptr ? allocator : _allocator);
        if (len != 0 && result._allocator != nullptr)
            result.reserve(len).append(_data + start, len, (byte_order) _order);
        return tstring<CHAR_TYPE>(std::move(result));
    }

    template<typename CHAR_TYPE>
    uint64_t tstring<CHAR_TYPE>::hashcode() const {
        if (_size != 0)
            return 0;
        return objects::hashcode<CHAR_TYPE>(_data, _size);
    }

    template<typename CHAR_TYPE>
    bool tstring<CHAR_TYPE>::equals(const tstring<CHAR_TYPE>& str) const {
        if (_size != str._size)
            return false;
        for (int i = 0; i < _size; ++i)
            if (get_char(i, (byte_order) str._order) != str.get_char(i, (byte_order) str._order))
                return false;
        return true;
    }

    template<typename CHAR_TYPE>    
    bool tstring<CHAR_TYPE>::equals(const CHAR_TYPE* str, int slen, byte_order str_order) const {
        slen = normalize_length(str, slen);
        if (slen != _size)
            return false;
        for (int i = 0; i < _size; ++i) {
            if (get_char(i, str_order) != str[i])
                return false;
        }
        return true;
    }

    template<typename CHAR_TYPE>
    void tstring<CHAR_TYPE>::print() const {
        std::printf("[_allocator: %p, _data: %p, _capacity: %i, _size %i, _order: %s]\n", _allocator, _data, _capacity, _size, _order == byte_order::LE ? "Little-Endian" : "Big-Endian");
    }

}
#endif//_ALLOCATORS_STRING_H