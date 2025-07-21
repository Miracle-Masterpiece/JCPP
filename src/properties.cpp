#include <cpp/lang/io/properties.hpp>
#include <cpp/lang/utils/date.hpp>

namespace jstd {

    properties::properties(tca::base_allocator* allocator) : 
    _allocator(allocator), 
    _props(allocator) {

    }

    bool properties::is_empty() const {
        return _props.is_empty();
    }

    void properties::set(const string& key, const string& value) {
        _props.put(key, value);
    }

    void properties::set(const char* key, const char* value) {
        tca::scope_allocator scope(_allocator);        
        _props.put(string(_allocator, key), string(_allocator, value));
    }

    const string* properties::get(const string& key) const {
        return _props.get(key);
    }

    const string& properties::get_or_default(const string& key, const string& _default) const {
        return _props.get_or_default(key, _default);
    }

    const string* properties::get(const char* key) const {
        return _props.get(string(_allocator, key));
    }

    void properties::save(ostream& out) const {
        const char new_line = '\n';
        const char assign   = '=';

        {
            char date_strbuf[64]{'#'};
            date now = date::now();
            int len = now.to_string(date_strbuf + 1, sizeof(date_strbuf) - 1);
            out.write(date_strbuf, len + 1);
            out.write(&new_line, 1);
        }

        for (const map_node<string, string>& entry : _props) {
            const string& key     = entry.get_key();
            const string& value   = entry.get_value();
            out.write(reinterpret_cast<const char*>(key.c_string()), key.length());
            out.write(&assign, 1);      //add "="
            out.write(reinterpret_cast<const char*>(value.c_string()), value.length());
            out.write(&new_line, 1);    //add "\n"
        }
    }

    void properties::load(istream& in) {
#ifndef NDEBUG
        if (_allocator == nullptr)
            throw_except<illegal_state_exception>("allocator must be != null");
#endif//NDEBUG
        u8string buffer(_allocator);
        buffer.reserve(128);

        u8string key(_allocator);
        key.reserve(32);
        
        u8string value(_allocator);
        value.reserve(32);

        int reader;
        char ch;
        while ((reader = in.read()) != -1) {
            if (reader == '#') {
                while ((reader = in.read()) != -1) {
                    if (reader == '\n')
                        break;
                }
                key.clear();
                value.clear();
                buffer.clear();
            }
            
            ch = (char) (reader & 0xff);
            if (reader == '=') {
                key.clear();
                key.append(buffer.c_string());
                buffer.clear();
            } 

            else if (reader == '\n') {
                if (buffer.length() != 0) {
                    value.append(buffer.c_string());
                    if (key.length() != 0 && value.length() != 0) {
                        key.trim();
                        value.trim();
                        _props.put(key, value);
                    }
                }
                
                buffer.clear();
                value.clear();
                key.clear();
            } 

            else {
                buffer.append(&ch, 1);
            }
        }
        if (key.length() != 0) {
            if (buffer.length() != 0) {
                value.append(buffer.c_string());
                key.trim();
                value.trim();
                _props.put(key, value);
            }
        }
    }
}