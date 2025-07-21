#include <cpp/lang/cstr.hpp>

namespace jstd {

    cstr::cstr(const char* s) : m_cstr(s), m_length(-1) {
        
    }

    cstr::operator const char* () const {
        return m_cstr;
    }
    
    int32_t cstr::length() const {
        if (m_length == -1) {
            if (m_cstr != nullptr)
                m_length = (int32_t) std::strlen(m_cstr);
            else
                m_length = 0;
        }
        return m_length;
    }

    const char& cstr::operator[] (int32_t idx) const {
#ifndef NDEBUG
        check_index(idx, length());
#endif//NDEBUG
        return m_cstr[idx];
    }

    bool cstr::equals(const cstr& s) const {
        int32_t len1 = length();
        int32_t len2 = s.length();
        if (len1 != len2)
            return false;
        for (int32_t i = 0, len = len1; i < len; ++i)
            if ((*this)[i] != s[i])
                return false;
        return true;
    }

    bool cstr::operator==(const cstr& s) const {
        return equals(s);
    }
    
    bool cstr::operator!=(const cstr& s) const {
        return !equals(s);
    }

    uint64_t cstr::hashcode() const {
        return objects::hashcode(m_cstr, m_length);
    }

    bool cstr::is_empty() const {
        return length() == 0;
    }

}