#include <cpp/lang/ustring.hpp>


namespace jstd {

    /*
     * ##########################################################################
     *                          U T F - 8
     * ##########################################################################
     */

    /*static*/ bool utf8_t::is_header(char ch) {
        if (ch < 0x7f) {
            return true;
        } else if ((ch & 0xE0) == 0xC0) {
            return true;
        } else if ((ch & 0xF0) == 0xE0) {
            return true;
        } else if ((ch & 0xF8) == 0xF0) {
            return true;
        }
        return false;
    }
    
    /*static*/ int utf8_t::symbol_size(char header) {
        if ((header & 0x80) == 0) 
            return 1;
        else if ((header & 0xE0) == 0xC0)
            return 2;
        else if ((header & 0xF0) == 0xE0)
            return 3;
        else if ((header & 0xF8) == 0xF0)
            return 4;
        else
            throw_except<utf_format_exception>("Character %x is not header!", (unsigned int) header);
        return 0;
    }
    
    /*static*/ codepoint_t utf8_t::chars_to_codepoint(const char* str) {
        int char_size = symbol_size(*str);
        if (char_size == 1) {
            return *str;
        }
        else if (char_size == 2) {
            codepoint_t ch1 = str[0] & 0x1F;
            codepoint_t ch2 = str[1] & 0x7F;
            return ch2 | (ch1 << 6);
        }
        else if (char_size == 3) {
            codepoint_t ch1 = str[0] & 0xF;
            codepoint_t ch2 = str[1] & 0x3F;
            codepoint_t ch3 = str[2] & 0x3F;
            return ch3 | (ch2 << 6) | (ch1 << 12);
        }
        else if (char_size == 4) {
            codepoint_t ch1 = str[0] & 0x7;
            codepoint_t ch2 = str[1] & 0x3F;
            codepoint_t ch3 = str[2] & 0x3F;
            codepoint_t ch4 = str[3] & 0x3F;
            return ch4 | (ch3 << 6) | (ch2 << 12) | (ch1 << 18);
        }
        else
            throw_except<utf_format_exception>("Character %x is not header!", (unsigned int) *str);
        return 0;
    }
    
    std::ostream& operator<<(std::ostream& out, const uint16_t* str) {
        if (str == nullptr)
            throw_except<null_pointer_exception>("str is null!");
        for (int i = 0; ;) {
            uint16_t ch = str[i++];
            if (utf16_t::is_low_surrogate(ch))
                throw_except<utf_format_exception>("Invalid UTF-16 sequence!: The symbol begins with a minor surrogate");
            if (ch == 0)
                break;
            codepoint_t cp = 0;
            if (utf16_t::is_high_surrogate(ch)) {
                uint16_t ch2 = str[i++];
                if (utf16_t::is_low_surrogate(ch2)) {
                    uint16_t _buf[2] = {ch, ch2};
                    cp = utf16_t::chars_to_codepoint(_buf, system::native_byte_order());
                } else {
                    throw_except<utf_format_exception>("Invalid UTF-16 sequence!: The second character of the surrogate pair is not a surrogate");
                }
            } else {
                cp = (codepoint_t) ch;
            }
            
            char buf[5];
            int len         = utf8_t::codepoint_to_chars(buf, cp);
            buf[len]        = '\0';
            out << buf;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const utf32_t* str) {
        if (str == nullptr)
            throw_except<null_pointer_exception>("str is null!");
        char buf[5];
        for (int i = 0; ;) {
            utf32_t ch = str[i++];
            if (ch == 0)
                break;
            int len = utf8_t::codepoint_to_chars(buf, (codepoint_t) ch);
            buf[len] = 0;
            out << buf;
        }
        return out;
    }

    /*static*/ int utf8_t::codepoint_to_chars(char buf[], codepoint_t codepoint) {
        if (codepoint < 0x7f){
            buf[0] = (unsigned char) codepoint;
            return 1;
        } else if (codepoint >= 0x80      && codepoint < 0x7ff) {
            char b1 = ((codepoint >> 6) & 0x1F) | 0xC0;
            char b2 = ((codepoint >> 0) & 0x3F) | 0x80;
            buf[0] = b1;
            buf[1] = b2;
            return 2;
        } else if (codepoint >= 0x800     && codepoint < 0xffff) {
            char b1 = ((codepoint >> 12) & 0xF)     | 0xE0;
            char b2 = ((codepoint >> 6)  & 0x3F)    | 0x80;
            char b3 = ((codepoint >> 0)  & 0x3F)    | 0x80;
            buf[0] = b1;
            buf[1] = b2;
            buf[2] = b3;
            return 3;
        } else if (codepoint >= 0x10000   && codepoint < 0x10ffff) {
            char b1 = ((codepoint >> 18)  & 0x7)  | 0xF0;
            char b2 = ((codepoint >> 12)  & 0x3F) | 0x80;
            char b3 = ((codepoint >> 6)   & 0x3F) | 0x80;
            char b4 = ((codepoint >> 0)   & 0x3F) | 0x80;
            buf[0] = b1;
            buf[1] = b2;
            buf[2] = b3;
            buf[3] = b4;
            return 4;
        }
        return -1;
    }

    template<>
    int tustring<char>::index_at(int idx) const {
        if (idx < 0)
            throw_except<illegal_argument_exception>("Index %i < 0", idx);
        
        int off         = 0;
        int itIndex     = 0;
        while (off < _size && idx != itIndex) {
            off += utf8_t::symbol_size(_data[off]);
            ++itIndex;
        }

        if (idx > itIndex) 
            throw_except<illegal_argument_exception>("Index %i > max %i", idx, itIndex);

        return off;
    }

    template<>
    int tustring<char>::codepoints_count() const {
        int len = 0;
        for (int i = 0; i < _size; ++len)
            i += utf8_t::symbol_size(_data[i]);
        return len;
    }
    
    /*
     * ##########################################################################
     *                          U T F - 16
     * ##########################################################################
     */

    /*static*/ bool utf16_t::is_high_surrogate(uint16_t ch) {
        return ch >= 0xd800 && ch <= 0xdbff;
    }

    /*static*/ bool utf16_t::is_low_surrogate(uint16_t ch) {
        return ch >= 0xdc00 && ch <= 0xdfff;
    }

    /*static*/ int utf16_t::symbol_size(uint16_t ch) {
        return is_high_surrogate(ch) ? 2 : 1;
    }

    // uint16_t read_with_ordering(const uint16_t* str, byte_order order) {
    //     uint16_t ch;
    //     if (order != system::native_byte_order()) {
    //         ch = utils::bswap<uint16_t>(*str);
    //     } 
    //     else {
    //         ch = str[0];
    //     }
    //     return ch;
    // }

    // void write_with_ordering(uint16_t* str, uint16_t ch, byte_order order) {
    //     if (order != system::native_byte_order()) {
    //         utils::copy_swap_memory(str, &ch, sizeof(utf16_t));
    //     } else {
    //         str[0] = ch;
    //     }
    // }

    /*static*/ codepoint_t utf16_t::chars_to_codepoint(const uint16_t* str, byte_order in) {
        uint16_t ch1 = utils::read_with_order<uint16_t>(str, in);
        if (is_high_surrogate(ch1)) {
            uint16_t ch2 = utils::read_with_order<uint16_t>(str + 1, in);
            if (is_low_surrogate(ch2)) {
                uint32_t c1 = ch1 - 0xD800;
                uint32_t c2 = ch2 - 0xDC00;
                return ((c1 << 10) | (c2 << 0)) + 0x10000;
            }   
        }
        return (codepoint_t) ch1;
    }

    /*static*/ int utf16_t::codepoint_to_chars(uint16_t buf[], codepoint_t cp, byte_order out) {
        if (cp < 0x10000) {
            utils::write_with_order<uint16_t>(buf, (uint16_t) cp, out);
            return 1;
        } else {
            cp -= 0x10000;
            uint16_t c1 = (uint16_t) (((cp >> 10) & 0x3ff) + 0xD800);
            uint16_t c2 = (uint16_t) (((cp >>  0) & 0x3ff) + 0xDC00);
            utils::write_with_order<uint16_t>(buf + 0, c1, out);
            utils::write_with_order<uint16_t>(buf + 1, c2, out);
            return 2;
        }
    }

    template<>
    template<>
    tustring<uint16_t> tustring<char>::recode(byte_order out_order, tca::base_allocator* allocator) const {
        if (allocator == nullptr)
            allocator = _allocator;

        if (_size == 0 || allocator == nullptr)
            return tustring<uint16_t>(allocator);

        tustring<uint16_t> result(allocator, nullptr, system::native_byte_order(), out_order);
        uint16_t buf[4];

        for (int i = 0; i < _size; ) {
            int char_size   = utf8_t::symbol_size(_data[i]);            
            int codepoint   = utf8_t::chars_to_codepoint(_data + i);
            int sz          = utf16_t::codepoint_to_chars(buf, codepoint, out_order);
            result.append(buf, sz, out_order);
            i += char_size;
        }
        
        return tustring<uint16_t>(std::move(result));
    }

    template<>
    codepoint_t tustring<char>::codepoint_at(int idx) const {
        check_index(idx, codepoints_count());
        if (_size == 0)
            return 0;
        return utf8_t::chars_to_codepoint(index_at(idx) + _data);
    }

    template<>
    template<>
    tustring<char> tustring<uint16_t>::recode(byte_order out, tca::base_allocator* allocator) const {
        if (allocator == nullptr)
            allocator = _allocator;
        if (_size == 0 || allocator == nullptr)
            return tustring<char>(allocator);

        tustring<char> result(allocator);

        char buf[4];
        for (int i = 0; i < _size; ) {
            int char_size   = utf16_t::symbol_size(utils::read_with_order<uint16_t>(_data + i, (byte_order) _order));
            int codepoint   = utf16_t::chars_to_codepoint(_data + i, (byte_order) _order);
            int sz          = utf8_t::codepoint_to_chars(buf, codepoint);
            result.append(buf, sz);
            i += char_size;
        }

        return tustring<char>(std::move(result));
    }

    template<>
    int tustring<uint16_t>::index_at(int idx) const {
        if (idx < 0)
            throw_except<illegal_argument_exception>("Index %i < 0", idx);
    
        byte_order native_order = system::native_byte_order();
        int off         = 0;
        int itIndex     = 0;
        while (off < _size && idx != itIndex) {
            uint16_t ch = get_char(off, native_order);
            if (utf16_t::is_high_surrogate(ch)) {
                off += 2;
            } else {
                off += 1;
            }
            ++itIndex;
        }

        if (idx > itIndex) 
            throw_except<illegal_argument_exception>("Index %i > max %i", idx, itIndex);

        return off;
    }

    template<>
    template<>
    tustring<utf32_t> tustring<char>::recode(byte_order out, tca::base_allocator* allocator) const {
        tca::base_allocator* alloc = allocator;
        if (alloc == nullptr)
            alloc = _allocator;
        if (_size == 0 || alloc == nullptr)
            return tustring<utf32_t>();
        tustring<utf32_t> result(alloc, nullptr, system::native_byte_order(), out);
        for (int i = 0; i < _size; ) {
            int char_size   = utf8_t::symbol_size(_data[i]);
            codepoint_t cp  = utf8_t::chars_to_codepoint(_data + i);
            i += char_size;
            utf32_t ch = {cp};
            result.append(&ch, 1);
        }
        return tustring<utf32_t>(std::move(result));
    }

    template<>
    template<>
    tustring<utf32_t> tustring<uint16_t>::recode(byte_order out, tca::base_allocator* allocator) const {
        tca::base_allocator* alloc = allocator;
        if (alloc == nullptr)
            alloc = _allocator;
        if (_size == 0 || alloc == nullptr)
            return tustring<utf32_t>();
        tustring<utf32_t> result(alloc, nullptr, system::native_byte_order(), out);
        
        byte_order system_order = system::native_byte_order();

        for (int i = 0; i < _size; ) {
            uint16_t ch = get_char(i, system_order);
            if (utf16_t::is_low_surrogate(ch))
                throw_except<utf_format_exception>("Invalid UTF-16 sequence!: The symbol begins with a minor surrogate");

            int char_size = utf16_t::symbol_size(ch);
            utf32_t utf32_char;
            if (utf16_t::is_high_surrogate(ch)) {
                uint16_t ch2 = get_char(i + 1, system_order);
                if (!utf16_t::is_low_surrogate(ch2))
                    throw_except<utf_format_exception>("Invalid UTF-16 sequence!: The second character of the surrogate pair is not a surrogate");
                utf32_char = utf16_t::chars_to_codepoint(_data + i, (byte_order) _order);
            } else {
                utf32_char = (uint32_t) ch;
            }
            result.append(&utf32_char, 1, system_order);
            i += char_size;
        }

        return tustring<utf32_t>(std::move(result));
    }

    template<>
    int tustring<uint16_t>::codepoints_count() const {
        if (_size == 0)
            return 0;
        int len = 0;
        const byte_order native_order = system::native_byte_order();
        for (int i = 0; i < _size; ++len) {
            uint16_t ch = get_char(i, native_order);
            if (utf16_t::is_high_surrogate(ch)) {
                if (i + 1 >= _size)
                    throw_except<utf_format_exception>("Invalid UTF16 data!");
                i += 2;
            } else {
                ++i;
            }
        }
        return len;
    }

    template<>
    codepoint_t tustring<uint16_t>::codepoint_at(int idx) const {
        check_index(idx, codepoints_count());
        return utf16_t::chars_to_codepoint(_data + index_at(idx), system::native_byte_order());
    }

    template<>
    int tustring<utf32_t>::index_at(int idx) const {
        return idx;
    }

    template<>
    int tustring<utf32_t>::codepoints_count() const {
        return _size;
    }

    template<>
    codepoint_t tustring<utf32_t>::codepoint_at(int idx) const {
        return get_char(idx, system::native_byte_order());
    }

    u16string make_utf16(const char* utf8_str, tca::base_allocator* allocator, byte_order out_order, int32_t len) {
        len = len < 0 ? (int32_t) std::strlen(utf8_str) : len;
        
        int u16_length = 0;
        {
            for (int i = 0; i < len; ) {
                int char_size = utf8_t::symbol_size(utf8_str[i]);
                i += char_size;
                if (char_size > 3) {
                    u16_length += 2;
                } else {
                    u16_length += 1;
                }
            }
        }

        u16string str(allocator, nullptr, system::native_byte_order(), out_order);
        str.reserve(u16_length);

        uint16_t tmp_buf[2];
        for (int i = 0; i < len ; ) {
            int char_size       = utf8_t::symbol_size(utf8_str[i]);
            codepoint_t cp      = utf8_t::chars_to_codepoint(utf8_str + i);
            int utf16_char_size = utf16_t::codepoint_to_chars(tmp_buf, cp, out_order);
            str.append(tmp_buf, utf16_char_size, out_order);
            i += char_size;
        }
        return u16string(std::move( str ));
    }


    u8string make_utf8(const uint16_t* utf16_str, tca::base_allocator* allocator, byte_order in_order, int len) {
        len = len < 0 ? u16string::strlen(utf16_str) : len;
        
        int utf8_length = 0;
        {
            for (int i = 0; i < len ; ) {
                uint16_t ch = utils::read_with_order<uint16_t>(utf16_str + i, in_order);
                ++i;
                codepoint_t cp;
                if (utf16_t::is_high_surrogate(ch)) {
                    uint16_t ch2 = utils::read_with_order<uint16_t>(utf16_str + i, in_order);
                    ++i;
                    uint16_t buf[2] {ch, ch2};
                    cp = utf16_t::chars_to_codepoint(buf, system::native_byte_order());
                } else {
                    cp = (uint32_t) ch;
                }
                char buf[4];
                int char_size = utf8_t::codepoint_to_chars(buf, cp);
                utf8_length += char_size;
            }
        }
        
        u8string result(allocator);
        result.reserve(utf8_length);
        for (int i = 0; i < len ; ) {
            uint16_t ch = utils::read_with_order<uint16_t>(utf16_str + i, in_order);
            ++i;
            codepoint_t cp;
            if (utf16_t::is_high_surrogate(ch)) {
                uint16_t ch2 = utils::read_with_order<uint16_t>(utf16_str + i, in_order);
                ++i;
                uint16_t buf[2] {ch, ch2};
                cp = utf16_t::chars_to_codepoint(buf, system::native_byte_order());
            } else {
                cp = (uint32_t) ch;
            }
            char buf[4];
            int char_size = utf8_t::codepoint_to_chars(buf, cp);
            result.append(buf, char_size);
        }

        return u8string( std::move(result) );
    }

    //UTF8 CHAR_SEQUENCE
    template<>
    tustring<char>::codepoint_sequence::codepoint_sequence(const char* chars, int32_t length, int32_t offset, byte_order str_order) : 
    m_chars(chars), m_length(length), m_offset(offset), m_cp(0), m_str_order(str_order) {
        if (length > 0 && offset == 0)
            m_cp = utf8_t::chars_to_codepoint(chars);
    }

    template<>
    typename tustring<char>::codepoint_sequence& tustring<char>::codepoint_sequence::operator++() {
        check_index(m_offset, m_length);
        int32_t char_size = utf8_t::symbol_size(m_chars[m_offset]);
        if (m_offset + char_size > m_length)
            throw_except<utf_format_exception>("Invalid UTF-8 string!");
        m_offset += char_size;
        m_cp     = utf8_t::chars_to_codepoint(m_chars + m_offset);
        return *this;
    }

    //UTF16 CHAR_SEQUENCE
    template<>
    tustring<uint16_t>::codepoint_sequence::codepoint_sequence(const uint16_t* chars, int32_t length, int32_t offset, byte_order str_order) : 
    m_chars(chars), m_length(length), m_offset(offset), m_cp(0), m_str_order(str_order) {
        if (length > 0 && offset == 0)
            m_cp = utf16_t::chars_to_codepoint(chars, str_order);
    }

    template<>
    typename tustring<uint16_t>::codepoint_sequence& tustring<uint16_t>::codepoint_sequence::operator++() {
        check_index(m_offset, m_length);
        int32_t char_size = utf16_t::symbol_size(m_chars[m_offset]);
        if (m_offset + char_size > m_length)
            throw_except<utf_format_exception>("Invalid UTF-16 string!");
        m_offset += char_size;
        m_cp     = utf16_t::chars_to_codepoint(m_chars + m_offset, m_str_order);
        return *this;
    }
    
}