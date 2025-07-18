#ifndef _JSTD_LANG_USTRING_H_H
#define _JSTD_LANG_USTRING_H_H
#include <cpp/lang/string.hpp>
#include <cpp/lang/types.hpp>
#include <iostream>

namespace jstd {


template<typename CHAR_TYPE>
class tustring;

typedef uint32_t codepoint_t;

/**
 * ################################################################
 *                        U T F - 8
 * ################################################################
 */
struct utf8_t {
    static bool         is_header(char ch);
    static int          symbol_size(char ch);
    static codepoint_t  chars_to_codepoint(const char* str);
    static int          codepoint_to_chars(char buf[], codepoint_t cp);
};

/**
 * ################################################################
 *                       U T F - 16
 * ################################################################
 */
struct utf16_t {
    static bool         is_high_surrogate(uint16_t ch);
    static bool         is_low_surrogate(uint16_t ch);
    static int          symbol_size(uint16_t ch);
    static codepoint_t  chars_to_codepoint(const uint16_t* str, byte_order in);
    static int          codepoint_to_chars(uint16_t buf[], codepoint_t cp, byte_order out);
};
std::ostream& operator<<(std::ostream& out, const uint16_t* str);

/**
 * ################################################################
 *                      U T F - 32
 * ################################################################
 */
struct utf32_t {
    uint32_t _ch;
    operator uint32_t() const { return _ch; }
    utf32_t& operator= (uint32_t ch) { _ch = ch; return *this;}
};

std::ostream& operator<<(std::ostream& out, const utf32_t* str);

typedef tustring<char>      u8string;
typedef tustring<uint16_t>  u16string;
typedef tustring<uint32_t>  u32string;
typedef u8string string;

namespace utf {
    const char UTF_16_BIG_ENDIAN_BOM[]    = {(char) 0xfe, (char) 0xff};
    const char UTF_16_LITTLE_ENDIAN_BOM[] = {(char) 0xff, (char) 0xfe};
    const char UTF_32_BIG_ENDIAN_BOM[]    = {(char) 0,    (char) 0,    (char) 0xfe, (char) 0xff};
    const char UTF_32_LITTLE_ENDIAN_BOM[] = {(char) 0xff, (char) 0xfe, (char) 0,    (char) 0};
}


template<typename CHAR_TYPE> 
class tustring : public tstring<CHAR_TYPE> {
public:
    tustring(tca::base_allocator* allocator = tca::get_scoped_or_default());
 
    /**
     * @deprecated in @since 1.1
     */
    tustring(tca::base_allocator* allocator, const CHAR_TYPE* str, byte_order in = system::native_byte_order(), byte_order out = system::native_byte_order());
    tustring(const CHAR_TYPE* str, byte_order in = system::native_byte_order(), byte_order out = system::native_byte_order(), tca::base_allocator* allocator = tca::get_scoped_or_default());
 
    tustring(const tustring<CHAR_TYPE>& str);
    tustring(tstring<CHAR_TYPE>&& str);
    tustring(tustring<CHAR_TYPE>&& str);
    tustring<CHAR_TYPE>& operator= (const tustring<CHAR_TYPE>& str);
    tustring<CHAR_TYPE>& operator= (tustring<CHAR_TYPE>&& str);
    tustring<CHAR_TYPE>& operator= (tstring<CHAR_TYPE>&& str);
    ~tustring();
    
    /**
     * Создаёт view с-строки.
     * Строка, созданная этой функцией, не может быть изменена.
     * Любая попытка изменить будет бросать исключение.
     * 
     * @param str
     *      С-Строка, которая будет обёрнута в объект.
     * 
     * @param length (Опционально)
     *      Длина С-строки не включая нулевой символ. 
     *      Если указано отрицательное значение, длина строки будет вычислена автоматически, но она должна оканчиваться нуль-терминатором.
     * 
     * @param order
     *      Порядок байт входящей строки.
     * 
     * @return
     *      Неизменяемая строка.
     * 
     * @since 1.1
     */
    static tustring<CHAR_TYPE> make_view(const CHAR_TYPE* str, int32_t length = -1, byte_order order = system::native_byte_order());

    /**
     * ==============================================================
     *              S Y M B O L  F U N CS
     * ==============================================================
     */
    
    /**
     * Возвращает реальный индекс из позиции кодовой точки или печатного символа.
     * 
     * @param codepoint_pos
     *      Индекс кодовой точки или печатного символа.
     * 
     * @note
     *      Такие кодировки, как UTF-8 или UTF-16 могут хранить свои символы в нескольких байтах или суррогатных пар.
     *      Где один байт или представление символа может не соответсвовать реальному положению.
     *      Так, например, в UTF-8 буква "р" в слове "Привет", имеет индекс 1, 
     *      но реальный индекс будет 2 из-за того, что для буквы "П" необходимо 2 байта.
     * 
     * @return
     *      Реальный индекс в многобайтовой последовательности.
     */
    int index_at(int codepoint_pos) const;
    
    /**
     * Возвращает длину в количестве печатных символов.
     * 
     * @note
     *      Данная функция отличается от функции tstring<CHAR_TYPE>::size() тем,
     *      что она возвращает кол-во именно печатных символов. 
     *      Так, например, если строка UTF-8 хранит 1 печатный символ из 4 байт, то данная функция вернёт 1.
     * 
     * @return
     *      Количество печатных символов.
     * 
     * @since 1.0
     */
    int codepoints_count() const;
    
    /**
     * Возвращает кодовую точку, находящуюся по байтовому индексу.
     * 
     * @param idx
     *      Индекс в последовательности символов. 
     *      Этот индекс является индексом внутри строки, а не индексом печатного символа.
     * 
     * @return
     *      Кодовая точка юникода.
     * 
     * @throws utf_format_exception
     *      Если последовательность символов не смога быть преобразована в кодовую точку.
     */
    codepoint_t codepoint_at(int idx) const;

    /**
     * Возвращает первую позицию кодовой точки.
     * 
     * @return
     *      позиция кодовой точки или -1 если кодовая точка не содержится в строке.
     *      
     */
    int32_t codepoint_index_of(codepoint_t cp) const;

    /**
     * Конвертирует строку из одной кодировки в другую.
     * 
     * @tparam OUT_CHAR_TYPE
     *      Тип символа, который указывает на кодировку. 
     *      char        -   UTF-8
     *      uint16_t    -   UTF-16
     *      uint32_t    -   UTF-32
     * 
     * @param out_order
     *      Порядок байт конвертированной строки.
     * 
     * @param allocator
     *      Аллокатор для выделения памяти под выходную строку.
     *
     * @since 1.0
     */
    template<typename OUT_CHAR_TYPE>
    tustring<OUT_CHAR_TYPE> recode(byte_order out_order = system::native_byte_order(), tca::base_allocator* allocator = tca::get_scoped_or_default()) const;

    /**
     * Позволяет итерироваться по кодовым точкам юникода в строке.
     * Поддерживает кодировки UTF-8, UTF-16(LE/BE), UTF-32(LE/BE)
     * 
     * @since 1.2
     */
    class codepoint_sequence {
        const CHAR_TYPE*    m_chars;
        int32_t             m_length;
        int32_t             m_offset;
        codepoint_t         m_cp;
        byte_order          m_str_order;
    public:
        codepoint_sequence();
        codepoint_sequence(const CHAR_TYPE* chars, int32_t length, int32_t offset, byte_order str_order);
        codepoint_sequence(const codepoint_sequence&);
        codepoint_sequence(codepoint_sequence&&);
        codepoint_sequence& operator= (const codepoint_sequence&);
        codepoint_sequence& operator= (codepoint_sequence&&);
        ~codepoint_sequence();
        const codepoint_t& operator* () const;
        codepoint_sequence& operator++();
        codepoint_sequence  operator++(int);
        bool operator!=(const codepoint_sequence&) const;
    };

    /**
     * @since 1.2
     */
    codepoint_sequence begin() const;
    
    /**
     * @since 1.2
     */
    codepoint_sequence end() const;
};
    
    template<typename CHAR_TYPE>
    tustring<CHAR_TYPE>::tustring(tca::base_allocator* allocator) : tstring<CHAR_TYPE>(allocator) {
    }
    
    template<typename CHAR_TYPE>
    tustring<CHAR_TYPE>::tustring(tca::base_allocator* allocator, const CHAR_TYPE* str, byte_order in, byte_order out) :
    tstring<CHAR_TYPE>(allocator, str, in, out) {
    }

    template<typename CHAR_TYPE>
    tustring<CHAR_TYPE>::tustring(const CHAR_TYPE* str, byte_order in, byte_order out, tca::base_allocator* allocator) :
    tustring<CHAR_TYPE>(allocator, str, in, out) {

    }
    
    template<typename CHAR_TYPE>
    tustring<CHAR_TYPE>::tustring(const tustring<CHAR_TYPE>& str) : tstring<CHAR_TYPE>(str) {
    }
    
    template<typename CHAR_TYPE>
    tustring<CHAR_TYPE>::tustring(tustring<CHAR_TYPE>&& str) : tstring<CHAR_TYPE>(std::move(str)) {
    }
    
    template<typename CHAR_TYPE>
    tustring<CHAR_TYPE>& tustring<CHAR_TYPE>::operator= (const tustring<CHAR_TYPE>& str) {
        tstring<CHAR_TYPE>::operator=(str);
        return *this;
    }
    
    template<typename CHAR_TYPE>
    tustring<CHAR_TYPE>& tustring<CHAR_TYPE>::operator= (tustring<CHAR_TYPE>&& str) {
        tstring<CHAR_TYPE>::operator=(std::move(str));
        return *this;
    }

    template<typename CHAR_TYPE>
    tustring<CHAR_TYPE>::tustring(tstring<CHAR_TYPE>&& str) : tstring<CHAR_TYPE>(std::move(str)) {

    }

    template<typename CHAR_TYPE>
    tustring<CHAR_TYPE>& tustring<CHAR_TYPE>::operator= (tstring<CHAR_TYPE>&& str) {
        tstring<CHAR_TYPE>::operator=(std::move(str));
        return *this;
    }

    template<typename CHAR_TYPE>
    /*static*/ tustring<CHAR_TYPE> tustring<CHAR_TYPE>::make_view(const CHAR_TYPE* str, int32_t length, byte_order order) {
        tustring<CHAR_TYPE> view;
        view._capacity          = view._size = length >= 0 ? length : tstring<CHAR_TYPE>::strlen(str);
        view._const_data        = str;
        view._order             = order;
        view._allocator         = nullptr;
        return view;
    }

    template<typename CHAR_TYPE>
    tustring<CHAR_TYPE>::~tustring() {}

    template<typename CHAR_TYPE>
    int32_t tustring<CHAR_TYPE>::codepoint_index_of(codepoint_t cp) const {
        int32_t pos = 0;
        for (const codepoint_t& entry : *this) {
            if (entry == cp)
                return pos;
            ++pos;
        }
        return -1;
    }
    
    /**
     * #########################################################
     *                      U T F - 8
     * #########################################################
     */
    template<>
    int tustring<char>::index_at(int idx) const;

    template<>
    int tustring<char>::codepoints_count() const;

    template<>
    codepoint_t tustring<char>::codepoint_at(int idx) const;

    /**
     * Создаёт строку UTF-16 из последовательности символов UTF-8.
     * 
     * @param utf8_str
     *      Строка в кодировке UTF-8 или ASKII
     * 
     * @param allocator
     *      Аллокатор для выделения памяти под возвращаемую строку.
     * 
     * @param out_order
     *      Порядок байт возвращаемой строки.
     * 
     * @param len
     *      Длина utf8_str (Опционально!)
     *      Если указано -1, длина вычисляется автоматически.
     * 
     * @return 
     *      Строка в кодировке UTF-16
     */
    u16string make_utf16(const char* utf8_str, tca::base_allocator* allocator, byte_order out_order = system::native_byte_order() , int len = -1);

    /**
     * Создаёт строку UTF-8 из последовательности символов UTF-16.
     * 
     * @param utf16_str
     *      Строка в кодировке UTF-16.
     * 
     * @param allocator
     *      Аллокатор для выделения памяти под возвращаемую строку.
     * 
     * @param in_order
     *      Порядок байт utf16_str.
     * 
     * @param len
     *      Длина строки utf16_str (Опционально!)
     *      Если указано -1, длина вычисляется автоматически.
     * 
     * @return
     *      Строка в кодировке UTF-&
     */
    u8string make_utf8(const uint16_t* utf16_str, tca::base_allocator* allocator, byte_order in_order = system::native_byte_order() , int len = -1);

    /**
     * #########################################################
     *                      U T F - 16
     * #########################################################
     */
    
    template<>
    int tustring<uint16_t>::index_at(int idx) const;

    template<>
    int tustring<uint16_t>::codepoints_count() const;

    template<>
    codepoint_t tustring<uint16_t>::codepoint_at(int idx) const;

    /**
     * #########################################################
     *                      U T F - 32
     * #########################################################
     */
    template<>
    int tustring<utf32_t>::index_at(int idx) const;

    template<>
    int tustring<utf32_t>::codepoints_count() const;

    template<>
    codepoint_t tustring<utf32_t>::codepoint_at(int idx) const;

    /**
     * #########################################################
     *                  D E C O D E  F U N C S
     * #########################################################
     */

    template<>
    template<>
    tustring<uint16_t> tustring<char>::recode(byte_order out, tca::base_allocator* allocator) const;

    template<>
    template<>
    tustring<utf32_t> tustring<char>::recode(byte_order out, tca::base_allocator* allocator) const;

    template<>
    template<>
    tustring<char> tustring<uint16_t>::recode(byte_order out, tca::base_allocator* allocator) const;

    template<>
    template<>
    tustring<utf32_t> tustring<uint16_t>::recode(byte_order out, tca::base_allocator* allocator) const;


/**
 * ===================================================================================================================
 *                                          CODEPOINT_SEQUENCE
 * ===================================================================================================================
 */
        template<typename CHAR_TYPE>
        tustring<CHAR_TYPE>::codepoint_sequence::codepoint_sequence() :
        m_chars(nullptr), m_length(0), m_offset(0), m_cp(0), m_str_order(system::native_byte_order())  {

        }

        template<typename CHAR_TYPE>
        tustring<CHAR_TYPE>::codepoint_sequence::codepoint_sequence(const CHAR_TYPE* chars, int32_t length, int32_t offset, byte_order str_order) : 
        m_chars(chars), m_length(length), m_offset(offset), m_cp(0), m_str_order(str_order) {
            if (length > 0 && offset == 0) {
                (*this)++;
                m_offset = 0;
            }
        }
        
        template<typename CHAR_TYPE>
        tustring<CHAR_TYPE>::codepoint_sequence::codepoint_sequence(const codepoint_sequence& cs) : 
        m_chars(cs.m_chars), m_length(cs.m_length), m_offset(cs.m_offset), m_cp(cs.m_cp), m_str_order(cs.m_str_order) {

        }
        
        template<typename CHAR_TYPE>
        tustring<CHAR_TYPE>::codepoint_sequence::codepoint_sequence(codepoint_sequence&& cs) :
        m_chars(cs.m_chars), m_length(cs.m_length), m_offset(cs.m_offset), m_cp(cs.m_cp), m_str_order(cs.m_str_order) {
            cs.m_chars  = nullptr;
            cs.m_length = 0;
            cs.m_offset = 0;
            cs.m_cp     = 0;
        }
        
        template<typename CHAR_TYPE>
        typename tustring<CHAR_TYPE>::codepoint_sequence& tustring<CHAR_TYPE>::codepoint_sequence::operator= (const codepoint_sequence& cs) {
            if (&cs != this) {
                m_chars     = cs.m_chars;
                m_length    = cs.m_length;
                m_offset    = cs.m_offset;
                m_cp        = cs.m_cp;
                m_str_order = cs.m_str_order;
            }
            return *this;
        }
        
        template<typename CHAR_TYPE>
        typename tustring<CHAR_TYPE>::codepoint_sequence& tustring<CHAR_TYPE>::codepoint_sequence::operator= (codepoint_sequence&& cs) {
            if (&cs != this) {
                m_chars     = cs.m_chars;
                m_length    = cs.m_length;
                m_offset    = cs.m_offset;
                m_cp        = cs.m_cp;
                m_str_order = cs.m_str_order;

                cs.m_chars  = nullptr;
                cs.m_length = 0;
                cs.m_offset = 0;
                cs.m_cp     = 0;
            }
            return *this;
        }
        
        template<typename CHAR_TYPE>
        tustring<CHAR_TYPE>::codepoint_sequence::~codepoint_sequence() {

        }
        
        template<typename CHAR_TYPE>
        const codepoint_t& tustring<CHAR_TYPE>::codepoint_sequence::operator* () const {
            check_index(m_offset, m_length);
            return m_cp;
        }
        
        template<typename CHAR_TYPE>
        typename tustring<CHAR_TYPE>::codepoint_sequence& tustring<CHAR_TYPE>::codepoint_sequence::operator++() {
            check_index(m_offset, m_length);
            m_cp = m_chars[++m_offset];
            return *this;
        }
        
        template<typename CHAR_TYPE>
        typename tustring<CHAR_TYPE>::codepoint_sequence tustring<CHAR_TYPE>::codepoint_sequence::operator++(int) {
            check_index(m_offset, m_length);
            codepoint_sequence sq = *this;
            ++(*this);
            return sq;
        }

        template<typename CHAR_TYPE>
        bool tustring<CHAR_TYPE>::codepoint_sequence::operator!=(const codepoint_sequence& sq) const {
            return m_offset != sq.m_offset;
        }

        //UTF8 CHAR_SEQUENCE
        template<>
        tustring<char>::codepoint_sequence::codepoint_sequence(const char* chars, int32_t length, int32_t offset, byte_order str_order);

        template<>
        typename tustring<char>::codepoint_sequence& tustring<char>::codepoint_sequence::operator++();

        //UTF16 CHAR_SEQUENCE
        template<>
        tustring<uint16_t>::codepoint_sequence::codepoint_sequence(const uint16_t* chars, int32_t length, int32_t offset, byte_order str_order);

        template<>
        typename tustring<uint16_t>::codepoint_sequence& tustring<uint16_t>::codepoint_sequence::operator++();

        template<typename CHAR_TYPE>
        typename tustring<CHAR_TYPE>::codepoint_sequence tustring<CHAR_TYPE>::begin() const {
            return tustring<CHAR_TYPE>::codepoint_sequence(this->_data, this->_size, 0, (byte_order) this->_order);
        }
    
        template<typename CHAR_TYPE>
        typename tustring<CHAR_TYPE>::codepoint_sequence tustring<CHAR_TYPE>::end() const {
            return tustring<CHAR_TYPE>::codepoint_sequence(this->_data, this->_size, this->_size, (byte_order) this->_order);
        }

/**
 * ===================================================================================================================
 *                                          HASH_CODE
 * ===================================================================================================================
 */
template<typename T>
struct hash_for;

template<typename T>
struct equal_to;

template<>
struct equal_to<u8string> {
    bool operator() (const u8string& i1, const u8string& i2) {
        return i1.equals(i2);
    }
};

template<>
struct hash_for<u8string> {
    uint64_t operator() (const u8string& i) {
        return i.hashcode();
    }
};

template<>
struct equal_to<u16string> {
    bool operator() (const u16string& i1, const u16string& i2) {
        return i1.equals(i2);
    }
};

template<>
struct hash_for<u16string> {
    uint64_t operator() (const u16string& i) {
        return i.hashcode();
    }
};

template<>
struct equal_to<u32string> {
    bool operator() (const u32string& i1, const u32string& i2) {
        return i1.equals(i2);
    }
};

template<>
struct hash_for<u32string> {
    uint64_t operator() (const u32string& i) {
        return i.hashcode();
    }
};

template<>
struct equal_to<tstring<char>> {
    bool operator() (const tstring<char>& i1, const tstring<char>& i2) {
        return i1.equals(i2);
    }
};

template<>
struct hash_for<tstring<char>> {
    uint64_t operator() (const tstring<char>& i) {
        return i.hashcode();
    }
};

template<>
struct equal_to<tstring<wchar_t>> {
    bool operator() (const tstring<wchar_t>& i1, const tstring<wchar_t>& i2) {
        return i1.equals(i2);
    }
};

template<>
struct hash_for<tstring<wchar_t>> {
    uint64_t operator() (const tstring<wchar_t>& i) {
        return i.hashcode();
    }
};

}

#endif//_JSTD_LANG_USTRING_H_H