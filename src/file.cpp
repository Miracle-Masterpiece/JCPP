#include <cpp/lang/io/file.hpp>
#include <cpp/lang/io/filesystem.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <cstring>
#include <algorithm>
#include <cstdarg>
#include <iostream>
#include <utility>

namespace jstd {

    file::file() {
        _path[0] = '\0';
    }

    file::file(const char* path, int path_length) {
        path_length = path_length < 0 ? std::strlen(path) : path_length;
        const int buf_size = sizeof(_path) - 1;
        int result_size = std::min(buf_size, path_length);
        std::memcpy(_path, path, result_size);
        _path[result_size] = '\0';
        filesystem::normalize_path(_path, result_size);
    }
    
    file::file(const file& f) {
        std::memcpy(_path, f._path, sizeof(_path));
    }
    
    file::file(file&& f) {
        std::memcpy(_path, f._path, sizeof(_path));
    }
    
    file& file::operator= (const file& f) {
        if (&f != this)
            std::memcpy(_path, f._path, sizeof(_path));
        return *this;
    }
    
    file& file::operator= (file&& f) {
        if (&f != this)
            std::memcpy(_path, f._path, sizeof(_path));
        return *this;
    }
    
    file::~file() {
        
    }
    
    bool file::exists() const {
        return filesystem::exists(_path);
    }

    bool file::is_file() const {
        return filesystem::is_file(_path);
    }

    bool file::is_dir() const {
        return filesystem::is_dir(_path);
    }

    uint64_t file::length() const {
        return filesystem::length(_path);
    }

    uint64_t file::last_modified() const {
        return filesystem::last_modified(_path);
    }

    bool file::set_last_modified(uint64_t ms) {
        return filesystem::set_last_modified(_path, ms);   
    }

    bool file::is_absolute_path() const {
        return filesystem::is_absolute(_path);
    }

    bool file::create_new_file() {
        return filesystem::create_new_file(_path);
    }

    bool file::mkdir() {
        return filesystem::mkdir(_path);
    }

    bool file::mkdirs() {
        return filesystem::mkdirs(_path);
    }

    bool file::remove() {
        return filesystem::remove(_path);
    }

    bool file::rename_to(const char* new_name, int new_name_length) {
        return filesystem::rename_to(_path, new_name, -1, new_name_length);
    }

    bool file::can_execute() const {
        return filesystem::can_execute(_path);
    }

    bool file::can_read() const {
        return filesystem::can_read(_path);
    }

    bool file::can_write() const {
        return filesystem::can_write(_path);
    }

    bool file::set_executable(bool on_off) {
        return filesystem::set_executable(_path, on_off);
    }

    bool file::set_readable(bool on_off) {
        return filesystem::set_readable(_path, on_off);
    }

    bool file::set_writable(bool on_off) {
        return filesystem::set_writable(_path, on_off);
    }

    const char* file::str_path() const {
        return _path;
    }

    int file::get_name(char buf[], int bufsize) const {
        char path[sizeof(_path)];
        std::memcpy(path, _path, sizeof(path));
        
        filesystem::normalize_path(path);

        const int len = std::strlen(path);
        int idx = 0;
        for (int i = len - 1; i >= 0; --i) {
            if (filesystem::is_separator(path[i])) {
                idx = i + 1;
                break;    
            }
        }
        
        int name_length = std::min((len - idx), bufsize - 1);
        
        if (name_length >= 0) {
            for (int i = 0; i < name_length; ++i)
                buf[i] = path[i + idx];
            buf[name_length] = '\0';
            return len;
        } 

        return 0;
    }

    array<file> file::list_files(file_filter* filter /*= nullptr*/, tca::base_allocator* allocator) const {
        accept_all_filter accept_all;
        if (filter == nullptr)
            filter = &accept_all;
        
        int32_t count_files = filesystem::count_files_in_directory(_path, filter);
        
        array<file> files(count_files, allocator);

        directory_iterator begin(_path);
        directory_iterator end;

        int32_t i = 0;
        char path_buf[filesystem::MAX_LENGTH_PATH];
        while (begin != end) {
            directory_entry entry = *begin;
            int path_length = std::strlen(entry.get_name());
            if (filter->apply(entry.get_name(), path_length)) {
                path_length = std::snprintf(path_buf, sizeof(path_buf), "%s/%s", _path, entry.get_name());
                files[i++]  = file(path_buf);
            }
            ++begin;
        }

        return array<file>(std::move(files));
    }

    /*static*/ file file::runtime_path() {
        char buf[filesystem::MAX_LENGTH_PATH];
        int sz = filesystem::runtime_path(buf, sizeof(buf));
        return file(buf, sz);
    }

    /*static*/ file file::make(const char* format, ...) {
        va_list args;
        char buf[filesystem::MAX_LENGTH_PATH];
        va_start(args, format);
        std::vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        return file(buf);
    }

    /*static*/ file file::make_absolute(const char* format, ...) {
        char rt_path[filesystem::MAX_LENGTH_PATH];
        int len = filesystem::runtime_path(rt_path, sizeof(rt_path));
        if (len + 1 > (int) sizeof(rt_path))
            throw_except<illegal_state_exception>("buffer is small!");
        rt_path[len++] = filesystem::FILE_SEPARATOR;
        int free_space = sizeof(rt_path) - len;
        {
            va_list args;
            va_start(args, format);
            std::vsnprintf(rt_path + len, free_space, format, args);
            va_end(args);
        }
        return file(rt_path);
    }

    file file::get_parent() const {
        char parent_path_buf[filesystem::MAX_LENGTH_PATH];
        filesystem::get_parent(_path, parent_path_buf, sizeof(parent_path_buf));
        return file(parent_path_buf);
    }

    int64_t file::hashcode() const {
        return objects::hashcode(_path);
    }

    bool file::equals(const file& f) const {
        const char* p1 = _path;
        const char* p2 = f._path;
        while (*p1 && *p2)
            if (*(p1++) != *(p2++))
                return false;
        return true;
    }
}