#ifndef _JSTD_LANG_USTRING_H_H
#define _JSTD_LANG_USTRING_H_H
#include <cpp/lang/string.hpp>
#include <cpp/lang/types.hpp>

namespace jstd {


template<typename CHAR_TYPE>
class tustring;

    
typedef uint32_t codepoint_t;

/**
 * ################################################################
 *                      U T F - 8
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
 *                      U T F - 16
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
typedef tustring<utf32_t>   u32string;
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
     * @deprecated
     */
    tustring(tca::base_allocator* allocator, const CHAR_TYPE* str, byte_order in = system::native_byte_order(), byte_order out = system::native_byte_order());
    tustring(const CHAR_TYPE* str, byte_order in = system::native_byte_order(), byte_order out = system::native_byte_order(), tca::base_allocator* allocator = nullptr);
 
    tustring(const tustring<CHAR_TYPE>& str);
    tustring(tstring<CHAR_TYPE>&& str);
    tustring(tustring<CHAR_TYPE>&& str);
    tustring<CHAR_TYPE>& operator= (const tustring<CHAR_TYPE>& str);
    tustring<CHAR_TYPE>& operator= (tustring<CHAR_TYPE>&& str);
    tustring<CHAR_TYPE>& operator= (tstring<CHAR_TYPE>&& str);
    ~tustring();
    
    /**
     * ==============================================================
     *              S Y M B O L  F U N CS
     * ==============================================================
     */
    
    int                     index_at(int idx) const;
    int                     length_chars() const;
    int                     codepoint_at(int idx) const;

    template<typename OUT_CHAR_TYPE>
    tustring<OUT_CHAR_TYPE> recode(tca::base_allocator* allocator = nullptr, byte_order out = system::native_byte_order()) const;
};
    
    template<typename CHAR_TYPE>
    tustring<CHAR_TYPE>::tustring(tca::base_allocator* allocator) : tstring<CHAR_TYPE>(allocator != nullptr ? allocator : tca::get_scoped_or_default()) {
    }
    
    template<typename CHAR_TYPE>
    tustring<CHAR_TYPE>::tustring(tca::base_allocator* allocator, const CHAR_TYPE* str, byte_order in, byte_order out) :
    tstring<CHAR_TYPE>(allocator, str, in, out) {
    }

    template<typename CHAR_TYPE>
    tustring<CHAR_TYPE>::tustring(const CHAR_TYPE* str, byte_order in, byte_order out, tca::base_allocator* allocator) :
    tustring<CHAR_TYPE>(allocator != nullptr ? allocator : tca::get_scoped_or_default(), str, in, out) {

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
    tustring<CHAR_TYPE>::~tustring() {}

    /**
     * #########################################################
     *                      U T F - 8
     * #########################################################
     */
    template<>
    int tustring<char>::index_at(int idx) const;

    template<>
    int tustring<char>::length_chars() const;

    template<>
    int tustring<char>::codepoint_at(int idx) const;

    inline u8string make_utf8(const char* str, tca::base_allocator* allocator) {
        return u8string(allocator, str);
    }

    u16string make_utf16(const char* utf8_str, tca::base_allocator* allocator, byte_order out_order = system::native_byte_order() , int len = -1);

    u8string make_utf8(const uint16_t* utf16_str, tca::base_allocator* allocator, byte_order in_order = system::native_byte_order() , int len = -1);

    /**
     * #########################################################
     *                      U T F - 16
     * #########################################################
     */
    template<>
    int tustring<uint16_t>::index_at(int idx) const;

    template<>
    int tustring<uint16_t>::length_chars() const;

    template<>
    int tustring<uint16_t>::codepoint_at(int idx) const;

    /**
     * #########################################################
     *                      U T F - 32
     * #########################################################
     */
    template<>
    int tustring<utf32_t>::index_at(int idx) const;

    template<>
    int tustring<utf32_t>::length_chars() const;

    template<>
    int tustring<utf32_t>::codepoint_at(int idx) const;

    /**
     * #########################################################
     *                  D E C O D E  F U N C S
     * #########################################################
     */

    template<>
    template<>
    tustring<uint16_t> tustring<char>::recode(tca::base_allocator* allocator, byte_order out) const;

    template<>
    template<>
    tustring<utf32_t> tustring<char>::recode(tca::base_allocator* allocator, byte_order out) const;

    template<>
    template<>
    tustring<char> tustring<uint16_t>::recode(tca::base_allocator* allocator, byte_order out) const;

    template<>
    template<>
    tustring<utf32_t> tustring<uint16_t>::recode(tca::base_allocator* allocator, byte_order out) const;


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