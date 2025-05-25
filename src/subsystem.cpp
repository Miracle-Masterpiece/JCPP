//#define PROJECT_ASSEMBLER 0x1
#ifdef PROJECT_ASSEMBLER

#include <cpp/lang/ustring.hpp>
#include <cpp/lang/io/file.hpp>
#include <cpp/lang/io/utility.hpp>
#include <cpp/lang/io/properties.hpp>

//input stream
#include <cpp/lang/io/ifstream.hpp>
#include <cpp/lang/io/idstream.hpp>
#include <cpp/lang/io/ibstream.hpp>
#include <cpp/lang/io/inflstream.hpp>

//output stream
#include <cpp/lang/io/ofstream.hpp>
#include <cpp/lang/io/odstream.hpp>
#include <cpp/lang/io/obstream.hpp>
#include <cpp/lang/io/deflstream.hpp>

#include <allocators/shared_allocator.hpp>
#include <allocators/malloc_free_allocator.hpp>
#include <allocators/inline_linear_allocator.hpp>
#include <allocators/os_allocator.hpp>
#include <allocators/ArrayList.h>

#include <iostream>

#include <cpp/lang/utils/arrays.h>

#ifdef __linux__
    #define LINUX_OS __linux__
#elif _WIN32
    #define WINDOWS_OS _WIN32
#endif

tca::os_allocator allocator;
tca::shared_allocator m_alloc(&allocator, tca::shared_allocator::BEST_FIT);
using namespace jstd;
using namespace tca;

tca::array_list<jstd::string> args_to_string(int argc, const char* args[]) {
    array_list<string> arguments(&m_alloc);
    for (int i = 1; i < argc; ++i) {
        int len = std::strlen(args[i]);
        if (len > 0) {
            arguments.add(make_utf8(args[i], &m_alloc));
        }
    }
    return array_list<string>(std::move(arguments));
}

class source_filter : public file_filter {
public:
    bool apply(const char* filename, int sz) const override {
        u8string tmp = make_utf8(filename, &m_alloc);
        if (tmp.ends_with(".cpp") || tmp.ends_with(".c") || tmp.ends_with(".c++") || 
            tmp.ends_with(".hpp") || tmp.ends_with(".h") || tmp.ends_with(".h++"))
            return true;
        return false;
    }
};

bool is_source(const char* filename) {
    string tmp(&m_alloc, filename);
    if (tmp.ends_with(".cpp") || tmp.ends_with(".c") || tmp.ends_with(".c++"))
        return true;
    return false;
}

void get_all_files(array_list<file>& all_files, const jstd::file& relative, hash_map<string, uint64_t>& mod) {
    const char* const* const files = relative.list(&m_alloc);
    if (files == nullptr)
        return;
    source_filter filter;
    for (const char* const* i = files; *i != nullptr; ++i) {
        file current_name(*i);
        file full = file::make("%s/%s", relative.str_path(), current_name.str_path());
        if (full.is_file()) {
            if (filter.apply(full.str_path(), std::strlen(full.str_path()))) {
                if (is_source(full.str_path()))
                    all_files.add(full);
                uint64_t last_mod = full.last_modified();
                mod.put(string(&m_alloc, full.str_path()), last_mod);
            }
        } else {
            get_all_files(all_files, full, mod);
        }
    }

    file::free_list(files, &m_alloc);
}

void parse_keys(array_list<string>& arguments, properties& keys) {
    const string key_src(&m_alloc, "-src");
    const string key_bin(&m_alloc, "-bin");
    const string key_out(&m_alloc, "-out");
    const string key_reset(&m_alloc, "-reset"); //Перекомпилировать все файлы, независимо от времени их изменения.
    const string key_disable_linker(&m_alloc, "-dlinker");
    const string key_log(&m_alloc, "-log");    //Вывести лог компиляции
    for (std::size_t i = 0; i < arguments.size(); ) {
        const string& arg = arguments.at(i);
        
        if (arg.equals(key_src)) {
            keys.set(key_src, arguments.at(i+1));
            arguments.remove_at(i);
            arguments.remove_at(i);
        }
        
        else if (arg.equals(key_bin)) {
            keys.set(key_bin, arguments.at(i+1));
            arguments.remove_at(i);
            arguments.remove_at(i);
        } 
        
        else if (arg.equals(key_reset)) {
            keys.set(key_reset, string(&m_alloc, "true"));
            arguments.remove_at(i);
        } 
        
        else if (arg.equals(key_disable_linker)) {
            keys.set(key_disable_linker, string(&m_alloc, "true"));
            arguments.remove_at(i);
        } 

        else if (arg.equals(key_log)) {
            keys.set(key_log, string(&m_alloc, "true"));
            arguments.remove_at(i);
        }

        else if (arg.equals(key_out)) {
            keys.set(key_out, arguments.at(i+1));
            arguments.remove_at(i);
            arguments.remove_at(i);
        }

        else {
            ++i;
        }
    }   
}

hash_map<string, uint64_t> load_last_mod_files() {
    hash_map<string, uint64_t> map(&m_alloc);
    file save("./proj.dat");
    if (save.exists()) {
        ifstream in(save);
        ibstream buf_stream(&in, &m_alloc);
        inflstream inflate(&buf_stream, &m_alloc);
        idstream data_stream(&inflate);
        
        int32_t count_files = data_stream.read<int32_t>();
    
        for (int32_t i = 0; i < count_files; ++i) {
            int32_t slen = data_stream.read<int32_t>();   
            string data(&m_alloc); 
            data.reserve(slen + 1);
            
            for (int32_t j = 0; j < slen; ++j) {
                char c = data_stream.read<char>();
                data.append(&c, 1);
            }
            uint64_t time = data_stream.read<uint64_t>();

            map.put(std::move(data), time);
        }
    }
    
    return hash_map<string, uint64_t>(std::move(map));
}

void save_last_mod_files(const hash_map<string, uint64_t>& map) {
    file save("./proj.dat");
    ofstream out(save);
    obstream buf_stream(&out, &m_alloc);
    deflstream deflate(&buf_stream, &m_alloc);
    odstream data_stream(&deflate);
    
    int32_t count_files = map.size();
    data_stream.write<int32_t>(count_files);

    for (const map_node<string, uint64_t>& entry : map) {
        int32_t slen = entry.get_key().length();   
        data_stream.write<int32_t>(slen);
        data_stream.write<char>(entry.get_key().c_string(), slen);
        data_stream.write<uint64_t>(entry.get_value());
    }
}

struct compile_time_info {
    file* m_file;
    int64_t m_compile_time;
    compile_time_info(jstd::file* f, int64_t time);
};

compile_time_info::compile_time_info(file* file, int64_t time) : m_file(file), m_compile_time(time) {

}

template<>
struct jstd::compare_to<compile_time_info> {
    int operator ()(const compile_time_info& a, const compile_time_info& b) const {
        return a.m_compile_time - b.m_compile_time;
    }
};


int main(int argc, const char* args[]) {
try {
    file rt_path = file(".");
    
#ifdef ENABLE_OUTPUT
    std::cout << "rt: " << rt_path.str_path() << std::endl;
#endif

    tca::array_list<string> arguments = args_to_string(argc, args);
    
    /**
     * Карта для хранения всех изменений файлов, которые так или иначе представляют исходный код на С/С++
     * .c .c++ .cpp, .h .h++ .hpp
     */
    hash_map<string, uint64_t> all_last_modified_source_files(&m_alloc);
    hash_map<string, uint64_t> last_modifiend_from_file = load_last_mod_files();

    properties keys(&m_alloc);
    parse_keys(arguments, keys);

    /**
     * Перекомпилировать все файлы, независимо от того, какие были изменены. Даже если они не были изменены с последней компиляции.
     */
    if (keys.get("-reset") != nullptr)
        last_modifiend_from_file.clear();

    string cmd_compile(&m_alloc);
    string space = make_utf8(" ", &m_alloc);
    
    for (std::size_t i = 0, size = arguments.size(); i < size; ++i) {
        cmd_compile.append(arguments.at(i)).append(space);
    }
    
    const string* bin = keys.get("-bin");
    if (bin == nullptr) {
        printf("-bin not exists!");
        exit(-1);
    }
    
#ifdef ENABLE_OUTPUT
    std::cout << cmd_compile.c_string() << std::endl;
#endif

    array_list<file> files(&m_alloc, 1024 * 4);
    const string* src = nullptr;
    {
        src = keys.get("-src");
        if (src == nullptr) {
            printf("-src not exists!");
            exit(-1);
        }
        get_all_files(files, file((const char*) src->c_string()), all_last_modified_source_files);
    }
    
    string system_compile_command_buffer(&m_alloc);
    string file_in_out_buffer(&m_alloc);
    string replace_buffer(&m_alloc);
    char file_name_buffer[io::constants::MAX_LENGTH_PATH];
    
#ifdef ENABLE_OUTPUT
    for (const map_node<string, uint64_t>& entry : all_last_modified_source_files) {
        std::cout << entry.get_key().c_string() << " = " << entry.get_value() << std::endl;
    }
#endif

    array_list<compile_time_info> compile_info_list(&m_alloc);

    string tmp(&m_alloc);
    for (std::size_t i = 0; i < files.size(); ++i) {
        {
            files.at(i).get_name(file_name_buffer, sizeof(file_name_buffer));
            replace_buffer.append(file_name_buffer); 
            replace_buffer.replace(".cpp", ".o").replace(".c++", ".o").replace(".c", ".o");
            file_in_out_buffer.append("-o ").append(bin->c_string()).append(replace_buffer).append(" ").append(files.at(i).str_path());
        }

        //std::cout << file_in_out_buffer.c_string() << std::endl;
        
        tmp.append(files.at(i).str_path());
        
        //std::cout << tmp.c_string() << std::endl;
        uint64_t* in_file_last_mod_from_file    = last_modifiend_from_file.get(tmp);
        uint64_t* source_last_mod               = all_last_modified_source_files.get(tmp);
        assert(source_last_mod != nullptr);

        if (in_file_last_mod_from_file == nullptr || source_last_mod == nullptr || *source_last_mod != *in_file_last_mod_from_file) {
            system_compile_command_buffer.append(cmd_compile).replace("%file%", file_in_out_buffer.c_string());
            std::cout << "Compiling: " << system_compile_command_buffer.c_string() << std::endl;
            
            int64_t last = system::nano_time();
                std::system(system_compile_command_buffer.c_string());
            int64_t now = system::nano_time();
            
            compile_info_list.add(compile_time_info(&files.at(i), (now - last) / 1000000.0));

            if (!last_modifiend_from_file.replace(tmp, *source_last_mod))
               last_modifiend_from_file.put(tmp, *source_last_mod);
        }

        tmp.clear();
        file_in_out_buffer.clear();
        system_compile_command_buffer.clear();
        replace_buffer.clear();
    }

    //return 0;

    save_last_mod_files(last_modifiend_from_file);
    
    /**
     * Отключение линкера.
     */
    if (keys.get("-dlinker") != nullptr)
        return 0;
    
    
    std::cout << "Linking process...\n";
    string linker_buffer(&m_alloc);
    for (std::size_t i = 0; i < files.size(); ++i) {
        file f = files.at(i);
        {
            files.at(i).get_name(file_name_buffer, sizeof(file_name_buffer));
            replace_buffer.append(file_name_buffer); 
            replace_buffer.replace(".cpp", ".o").replace(".c++", ".o").replace(".c", ".o");
        }
        linker_buffer.append(bin->c_string()).append(replace_buffer).append(" ");
        replace_buffer.clear();
    }

    //std::cout << linker_buffer.c_string() << std::endl;

    string out_name(&m_alloc);
#ifdef WINDOWS_OS
    out_name.append("-o ").append(keys.get_or_default(string(&m_alloc, "-out"), string(&m_alloc, "a.exe")));
#elif LINUX_OS
    out_name.append("-o ").append(keys.get_or_default(string(&m_alloc, "-out"), string(&m_alloc, "a.out")));
#endif

    replace_buffer.append(cmd_compile).replace("%file%", linker_buffer.c_string()).replace("-c", out_name.c_string());
    std::cout << "Link all\n" << replace_buffer.c_string() << "\n";
    std::system(replace_buffer.c_string());

    //const char* str = u8_string(hello);
    //std::cout << str << std::endl;

    // m_alloc.print_log();
    //g++ -c -src C:/src/ -bin C:/bin/ %file% -lws2_32

    if (compile_info_list.size() > 0) {
        jstd::utils::intersect_sort(compile_info_list.data(), compile_info_list.size());
        for (std::size_t i = 0; i < compile_info_list.size(); ++i) {
            const compile_time_info* info = &compile_info_list.at(i);
            std::cout << info->m_file->str_path() << " = " << (info->m_compile_time) << " ms\n";
        }
    }

} catch (const throwable& t) {
    std::cout << t.cause() << "\n";
}
}
#endif


#include <iostream>
#include <allocators/shared_allocator.hpp>
#include <allocators/os_allocator.hpp>
#include <cpp/lang/ustring.hpp>
#include <cpp/lang/utils/array_list.hpp>
#include <cpp/lang/io/ifstream.hpp>
#include <cpp/lang/io/ofstream.hpp>
#include <cpp/lang/cstr.hpp>
#include <cpp/lang/utils/hash_map.hpp>
#include <cassert>
#include <cpp/lang/stacktrace/stacktrace.hpp>

tca::os_allocator os_alloc;
tca::shared_allocator m_alloc(&os_alloc, tca::shared_allocator::BEST_FIT);

// enum token_type {
//     DEFINE,
//     LEFT_BRACKET, RIGHT_BRACKET, // ( )
//     IDENTIFICATOR,
//     VOID,
//     SEMICOLON, // ;
//     TYPEDEF
// };


class token_type {
    static int global_ids;
    const char* m_name;
    int         m_id;
public:
    token_type();
    token_type(const char* name);
    static const token_type IDENTIFICATOR;
    static const token_type NUMBER;
    const char* name() const;
    bool operator== (const token_type& t) const;
    bool operator!= (const token_type& t) const;
};
    /*static*/ int token_type::global_ids = 0;
    /*static*/ const token_type token_type::IDENTIFICATOR("IDENTIFICATOR");
    /*static*/ const token_type token_type::NUMBER("NUMBER");
    
    token_type::token_type() : m_name(nullptr), m_id(-1) {

    }

    token_type::token_type(const char* name) : m_name(name), m_id(global_ids++) {

    }

    bool token_type::operator== (const token_type& t) const {
        return m_id == t.m_id;
    }

    bool token_type::operator!= (const token_type& t) const {
        return m_id != t.m_id;
    }

    const char* token_type::name() const {
        return m_name;
    }

struct token {
    token_type      m_type;
    jstd::string    m_value;
};

struct tokenizer {
    jstd::array_list<token>                 m_tokens;
    jstd::string                            m_input;
    jstd::hash_map<jstd::cstr, token_type>  m_map;
    int32_t                                 m_offset;
    tca::base_allocator*                    m_allocator;
    
    tokenizer();
    tokenizer(tca::base_allocator* allocator, jstd::string& input);

    bool is_token(char c) const;
    token_type get_token(char c) const;

    char next();
    bool has_next() const;
    void tokenize();
    void create_token(const char* tokstr, const token_type& tok);
    jstd::array_list<token>& token_list();
};

    tokenizer::tokenizer() : m_tokens(), m_input(), m_map(), m_offset(0), m_allocator(nullptr) {

    }
    
    tokenizer::tokenizer(tca::base_allocator* allocator, jstd::string& input) : 
    m_tokens(allocator), m_input(input), m_map(allocator), m_offset(0), m_allocator(allocator) {
        
    }

    void tokenizer::create_token(const char* tokstr, const token_type& tok) {
        m_map.put(tokstr, tok);
    }

    char tokenizer::next() {
        if (m_offset == m_input.length())
            return 0;
        return m_input.char_at(m_offset++);
    }
    
    bool tokenizer::has_next() const {
        return m_offset < m_input.length();
    }

    bool is_space(char c) {
        return c <= 32;
    }

    bool is_number(char c) {
        return c >= '0' && c <= '9';
    }

    bool is_hex_number(char c) {
        return (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
    }

    bool is_letter(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    jstd::array_list<token>& tokenizer::token_list() {
        return m_tokens;
    }

    bool tokenizer::is_token(char c) const {
        char str_buf[2];
        str_buf[0] = c;
        str_buf[1] = '\0';
        return m_map.contains(str_buf);
    }

    token_type tokenizer::get_token(char c) const {
        char str_buf[2];
        str_buf[0] = c;
        str_buf[1] = '\0';
        assert(m_map.contains(str_buf) != false);
        return *m_map.get(str_buf);
    }

    void tokenizer::tokenize() {
        if (!has_next())
            return;
        
        jstd::string buffer(m_allocator);
        buffer.reserve(1024);

        char c = next();
        
        while (has_next()) {
        
            if (is_space(c)) {
                c = next();
                continue;;
            } 
            
            if (is_number(c)) {

                while (is_number(c) || is_hex_number(c) || c == 'x') {
                    buffer.append(&c, 1);
                    c = next();
                }

                m_tokens.add(token{.m_type = token_type::NUMBER, .m_value = buffer});
                 
            } else {
                
                if (is_token(c)) {
                    buffer.append(&c, 1);
                    c = next();
                } else {
                    do {
                        buffer.append(&c, 1);
                        c = next();
                    } while (!is_space(c) && !is_token(c));
                }

                if (m_map.contains(buffer.c_string())) {
                    token_type tp = *m_map.get(buffer.c_string());
                    m_tokens.add(token{.m_type = tp, .m_value = buffer});
                } else {
                    m_tokens.add(token{.m_type = token_type::IDENTIFICATOR, .m_value = buffer});
                }
            }
            buffer.clear();
        }
    }

    jstd::string read_all_file(tca::base_allocator* allocator, const char* path) {
        jstd::file f(path);
        jstd::ifstream in(f);
        jstd::string data(&m_alloc);
        data.reserve(in.available());

        while (in.available() > 0) {
            char c = (char) in.read();
            data.append(&c, 1);
        }

        return data;
    }

    void save_to_file(const jstd::string& str, const char* path) {
        jstd::file f(path);
        jstd::ofstream out(f, false);
        out.write(str.c_string(), str.length());
        out.flush();
        out.close();
    }

    /*static*/ const token_type DEFINE("DEFINE");
    /*static*/ const token_type LEFT_BRACKET("LEFT_BRACKET");
    /*static*/ const token_type RIGHT_BRACKET("RIGHT_BRACKET");
    /*static*/ const token_type VOID("VOID");
    /*static*/ const token_type SEMICOLON("SEMICOLON");
    /*static*/ const token_type TYPEDEF("TYPEDEF");
    /*static*/ const token_type STAR("STAR");
    /*static*/ const token_type COMMA("COMMA");
    /*static*/ const token_type CONST("CONST");
    /*static*/ const token_type GL_API("GL_API");

    namespace types
    {
        /*static*/ const token_type GL_ENUM("GL_ENUM");
        /*static*/ const token_type GL_BOOLEAN("GL_BOOLEAN");
        /*static*/ const token_type GL_BIT_FIELD("GL_BIT_FIELD");
        /*static*/ const token_type GL_VOID("GL_VOID");
        /*static*/ const token_type GL_BYTE("GL_BYTE");
        /*static*/ const token_type GL_UBYTE("GL_UBYTE");
        /*static*/ const token_type GL_SHORT("GL_SHORT");
        /*static*/ const token_type GL_USHORT("GL_USHORT");
        /*static*/ const token_type GL_INT("GL_INT");
        /*static*/ const token_type GL_UINT("GL_UINT");
        /*static*/ const token_type GL_FLOAT("GL_FLOAT");
        /*static*/ const token_type GL_DOUBLE("GL_DOUBLE");
        /*static*/ const token_type GL_CHAR("GL_CHAR");
        /*static*/ const token_type GL_SIZEI("GL_SIZEI");
        /*static*/ const token_type GL_INTPTR("GL_INTPTR");
        /*static*/ const token_type GL_SIZEIPTR("GL_SIZEIPTR");
        /*static*/ const token_type GL_SYNC("GL_SYNC");
        /*static*/ const token_type GL_INT64("GL_INT64");
        /*static*/ const token_type GL_UINT64("GL_UINT64");
        /*static*/ const token_type TIME_T("TIME_T");

    namespace funcs {
        /*static*/ const token_type GLDEBUGPROC("GLDEBUGPROC");
    }
        

    }

    struct func_param {
        jstd::string m_type;
        jstd::string m_name;
        func_param(tca::base_allocator* allocator) : m_type(allocator), m_name(allocator) {

        }
    };

    struct glad_func {
        token* m_name;
        token* m_glad_name;
        jstd::array_list<token*> m_return;
        jstd::array_list<func_param> m_params;
        glad_func(tca::base_allocator* allocator) : m_name(nullptr), m_glad_name(nullptr), m_return(allocator), m_params(allocator) {

        }
        void print() const;
    };

    void glad_func::print() const {
        for (int i = 0; i < m_return.size(); ++i) {
            std::cout << m_return.at(i)->m_value.c_string() << " ";
        }
        
        if (m_name != nullptr)
            std::cout << m_name->m_value.c_string();

        std::cout << "(";
        for (int i = 0; i < m_params.size(); ++i) {
            const func_param& param = m_params.at(i);
            if (param.m_type.length() > 0)
                std::cout << param.m_type.c_string();
            if (param.m_name.length() > 0)
                std::cout  << param.m_name.c_string();
            if (i + 1 != m_params.size())
                std::cout << ", ";
        }
        std::cout << ")\n";
    }

    struct parser {
        tca::base_allocator*        m_allocator;
        jstd::array_list<token>*    m_tokens;
        int32_t                     m_offset;
        jstd::string                m_result;

        parser();
        parser(tca::base_allocator* allocator, jstd::array_list<token>* tokens);
        
        bool has_next() const;
        token* next();

        token* current_token();
        void next_token();
        
        void parse(bool header, const char* api, const char* call);

        jstd::string parse_header(const char* api, const char* calldecl);
        jstd::string parse_source(const char* api, const char* calldecl);
        jstd::string parse_constants();

        void parse_ptr_to_func(glad_func& func);
        void parse_ret_type(glad_func& func);
        void parse_typedef(glad_func& func);
        void parse_args(glad_func& func);
        
        void parse_define(glad_func& func);
    };

    parser::parser() : m_allocator(nullptr), m_tokens(), m_offset(0), m_result() {

    }
    
    parser::parser(tca::base_allocator* allocator, jstd::array_list<token>* tokens) :
    m_allocator(allocator), m_tokens(tokens), m_offset(0), m_result(allocator) {

    }

    bool parser::has_next() const {
        return m_offset + 1 < m_tokens->size();
    }
    
    token* parser::next() {
        if (m_offset >= m_tokens->size())
            return nullptr;
        return &(m_tokens->at(m_offset++));
    }

    token* parser::current_token() {
        if (m_offset >= m_tokens->size())
            return nullptr;
        return &m_tokens->at(m_offset);
    }
    
    void parser::next_token() {
        ++m_offset;
    }

    void parser::parse_ret_type(glad_func& func) {
        token* tok = current_token();
        do {
            func.m_return.add(tok);
            next_token();
            tok = current_token();
        } while(tok != nullptr && tok->m_type != LEFT_BRACKET);
    }

    void parser::parse_ptr_to_func(glad_func& func) {
        token* tok = current_token();
        do {
            next_token();
            tok = current_token();
        } while(tok != nullptr && tok->m_type != LEFT_BRACKET);
    }

    void parser::parse_args(glad_func& func) {
        token* tok = current_token();
        func_param param(m_allocator);
        do {
            next_token();
            tok = current_token();
            if (tok->m_type == COMMA || tok->m_type == RIGHT_BRACKET) {
                func.m_params.add(std::move(param));
                param = func_param(m_allocator);
            } else if (tok->m_type == token_type::IDENTIFICATOR) {
                param.m_name = tok->m_value;
            } else {
                param.m_type.append(tok->m_value); 
                if (tok->m_type != VOID)
                    param.m_type.append(" ");
            }
        } while(tok != nullptr && tok->m_type != SEMICOLON);
    }

    void parser::parse_typedef(glad_func& func) {
        token* tok = current_token();
        
        assert(tok != nullptr && tok->m_type == TYPEDEF);
        next_token();   //убираем typedef
        
        parse_ret_type(func);        
        assert(current_token()->m_type == LEFT_BRACKET);

        parse_ptr_to_func(func);
        assert(current_token()->m_type == LEFT_BRACKET);
        
        parse_args(func);
        assert(current_token()->m_type == SEMICOLON);
    }

    void parser::parse_define(glad_func& func) {
        token* tok = current_token();
        assert(tok != nullptr && tok->m_type == DEFINE);
        next_token();   //#define


        tok = current_token(); //name
        func.m_name = tok;

        next_token();

        tok = current_token(); //glad_name
        func.m_glad_name = tok;
    }

    void fill_func_param(jstd::string& buffer, const glad_func& func, bool add_type) {
        buffer.append("(");
        for (int i = 0; i < func.m_params.size(); ++i) {
            if (add_type)
                buffer.append(func.m_params.at(i).m_type);
            buffer.append(func.m_params.at(i).m_name);
            if (i + 1 != func.m_params.size())
                buffer.append(", ");
        }
        buffer.append(")");
    }

    jstd::string parser::parse_header(const char* api, const char* calldecl) {
        parse(true, api, calldecl);
        m_offset = 0;
        jstd::string result = m_result;
        m_result.clear();
        return result;
    }
    
    jstd::string parser::parse_source(const char* api, const char* calldecl) {
        parse(false, api, calldecl);
        m_offset = 0;
        jstd::string result = m_result;
        m_result.clear();
        return result;
    }

    void parser::parse(bool is_header, const char* api, const char* call) {
        //typedef void (APIENTRYP PFNGLCULLFACEPROC)(GLenum mode);
        
        if (!is_header) {
            m_result.append("#include<glad/glad.h>");
        }

        m_result.append("\nnamespace gl\n{\n");


        jstd::string buffer(m_allocator);
        buffer.reserve(128);

        glad_func current_func(m_allocator);
        token* tok = nullptr;
        
        bool make_func = false;
        while ((tok = current_token()) != nullptr) {
            make_func = false;
            if (tok->m_type == TYPEDEF) {
                current_func = glad_func(m_allocator);
                parse_typedef(current_func);
            } else if (tok->m_type == DEFINE) {
                parse_define(current_func);
                make_func = true;
            } else {
                next_token();
            }
            //std::cout << tok->m_value.c_string() << std::endl;

            if (make_func) {
                if (!is_header) {
                    buffer.append("\n#undef ").append(current_func.m_name->m_value).append("\n");
                } else {
                    buffer.append("\n\n    ");
                }
                
                //API
                if (is_header)
                    buffer.append(api).append(" ");
                
                //return type
                for (int i = 0; i < current_func.m_return.size(); ++i) {
                    buffer.append(current_func.m_return.at(i)->m_value);
                    if (i + 1 != current_func.m_return.size())
                        buffer.append(" ");
                }
                
                //call_decl
                //if (is_header)
                    buffer.append(" ").append(call);

                //func name
                buffer.append(" ").append(current_func.m_name->m_value);
                
                //params
                fill_func_param(buffer, current_func, true);

                if (!is_header) {
                    //func body
                    buffer.append(" {\n    ");
                

                    bool need_ret = false;
                    if (current_func.m_return.at(0)->m_type != VOID)
                        need_ret = true;
                    else if (current_func.m_return.size() > 1)
                        need_ret = true;

                    buffer.append(need_ret ? "return " : "");
                    buffer.append(current_func.m_glad_name->m_value);
                    fill_func_param(buffer, current_func, false);
                    buffer.append(";\n");
                    
                    buffer.append("}");

                    std::cout << buffer.c_string() << std::endl;    

                    m_result.append(buffer.c_string(), buffer.length());
                } else {
                    buffer.append(";");
                    m_result.append(buffer.c_string(), buffer.length());
                }
                
                buffer.clear();
            }

        }   

        m_result.append("\n}//namespace gl");

        std::cout << m_result.c_string() << std::endl;
    }

    jstd::string parser::parse_constants() {
        
        token* tok = nullptr;
        bool make_func = false;
        while ((tok = current_token()) != nullptr) {
            if (tok->m_type == DEFINE) {
                next_token();
                token* name = current_token();
                next_token();
                token* value = current_token();
                m_result.append("const int ").append(name->m_value).append(" = ").append(value->m_value).append(";\n");
            } else {
                next_token();
            }
        }
        
        jstd::string result = m_result;
        m_result.clear();
        return result;
    }

int main0() {
    jstd::string s = read_all_file(&m_alloc, "./gl_test.txt");
    //std::cout << s.c_string() << std::endl;

    tokenizer token_generator(&m_alloc, s);
    token_generator.create_token("#define", DEFINE);
    token_generator.create_token("(", LEFT_BRACKET);
    token_generator.create_token(")", RIGHT_BRACKET);
    token_generator.create_token("void", VOID);
    token_generator.create_token("typedef", TYPEDEF);
    token_generator.create_token(";", SEMICOLON);
    token_generator.create_token("*", STAR);
    token_generator.create_token(",", COMMA);
    token_generator.create_token("const", CONST);
    token_generator.create_token("GLAPI", GL_API);

    token_generator.create_token("GLenum", types::GL_ENUM);
    token_generator.create_token("GLboolean", types::GL_BOOLEAN);
    token_generator.create_token("GLbitfield", types::GL_BIT_FIELD);
    token_generator.create_token("GLvoid", types::GL_VOID);
    token_generator.create_token("GLbyte", types::GL_BYTE);
    token_generator.create_token("GLubyte", types::GL_UBYTE);
    token_generator.create_token("GLshort", types::GL_SHORT);
    token_generator.create_token("GLushort", types::GL_USHORT);
    token_generator.create_token("GLint", types::GL_INT);
    token_generator.create_token("GLuint", types::GL_UINT);
    token_generator.create_token("GLfloat", types::GL_FLOAT);
    token_generator.create_token("GLdouble", types::GL_DOUBLE);
    token_generator.create_token("GLchar", types::GL_CHAR);
    token_generator.create_token("GLsizei", types::GL_SIZEI);
    token_generator.create_token("GLintptr", types::GL_INTPTR);
    token_generator.create_token("GLsizeiptr", types::GL_SIZEIPTR);
    token_generator.create_token("GLsync", types::GL_SYNC);
    token_generator.create_token("GLint64", types::GL_INT64);
    token_generator.create_token("GLuint64", types::GL_UINT64);
    token_generator.create_token("time_t", types::TIME_T);

    token_generator.create_token("GLDEBUGPROC", types::funcs::GLDEBUGPROC);

    token_generator.tokenize();

    parser parser(&m_alloc, &token_generator.token_list());
    
    jstd::string header = parser.parse_header("LWCPPGL_API", "LWCPPGL_CALL");
    jstd::string source = parser.parse_source("LWCPPGL_API", "LWCPPGL_CALL");
    save_to_file(header, "./gl.h");
    save_to_file(source, "./gl.cpp");

    // jstd::string header_constants = parser.parse_constants();
    // save_to_file(header_constants, "./gl_constants.hpp");

    //m_alloc.print_log();

}


void c() {
    jstd::throw_except<jstd::null_pointer_exception>("pointer is null!");
}

void b() {
    c();
}

void a() {
    b();
}

#include <cpp/lang/concurrency/thread.hpp>
#include <cpp/lang/concurrency/mutex.hpp>
#include <cstdarg>
#include <cpp/lang/utils/audio/wav_data.hpp>

using namespace jstd;

class mythread : public thread {
public:
    void run() override {
        throw_except<null_pointer_exception>("ptr is null");
    }
};

int main() {
    try {
        ifstream in("./test.wav");
        wav_data music(&in, &m_alloc);
        std::cout << obj_to_cstr_buf(music) << "\n";
    } catch(const jstd::throwable& e) {
        std::cout << e.cause() << "\n";
        e.print_stack_trace();
    }
    std::cout << "end" << std::endl;
}