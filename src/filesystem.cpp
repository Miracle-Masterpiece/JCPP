//Вероятно, это самый худший файл во всём проекте. Fucking shit...

#include <cpp/lang/io/filesystem.hpp>
#include <cpp/lang/ustring.hpp>
#include <cpp/lang/array.hpp>
#include <allocators/inline_linear_allocator.hpp>

#if defined(_WIN32)
    #include <sys/types.h>
    #include <sys/stat.h>
    #define CAN_READ 	            _S_IREAD		//Можно читать
	#define CAN_WRITE 	            _S_IWRITE	    //Можно писать
	#define CAN_EXECUTE 	        _S_IEXEC 	    //Можно исполнять
#elif defined(__linux__)
    #include <sys/stat.h>
    #include <fcntl.h> 
    #include <unistd.h>
    #include <utime.h>
    #include <cerrno>
    #define READ_OWNER 		        S_IRUSR 	//Чтение для владельца
	#define WRITE_OWNER 		    S_IWUSR 	//Запись для владельца
	#define EXECUTABLE_OWNER 	    S_IXUSR	    //Выполнение для владельца

	#define READ_GROUP 		        S_IRGRP 	//Чтение для группы
	#define WRITE_GROUP 		    S_IWGRP 	//Запись для группы
	#define EXECUTABLE_GROUP 	    S_IXGRP	    //Выполнение для группы

	#define READ_OTHER 		        S_IROTH 	//Чтение для остальных
	#define WRITE_OTHER 		    S_IWOTH 	//Запись для остальных
	#define EXECUTABLE_OTHER 	    S_IXOTH	    //Выполнение для остальных

	#define CAN_READ 			    R_OK
	#define CAN_WRITE 			    W_OK
	#define CAN_EXECUTE 		    X_OK
    
    #define LINUX_READ              (READ_OWNER | READ_GROUP | READ_OTHER)
    #define LINUX_WRITE             (WRITE_OWNER | WRITE_GROUP | WRITE_OTHER)
    #define LINUX_EXECUTE           (EXECUTABLE_OWNER | EXECUTABLE_GROUP | EXECUTABLE_OTHER)

    #define READ_ONLY               ((READ_OWNER | READ_GROUP | READ_OTHER) | (EXECUTABLE_OWNER | EXECUTABLE_GROUP | EXECUTABLE_OTHER))

    #define PERMISSION_ALL          (LINUX_READ | LINUX_WRITE | LINUX_EXECUTE)
#endif

#define null nullptr
#define _tcall_
#define _checkIndex_(idx, len) (idx)
#define normalize_length(str, len) (len < 0 ? std::strlen(str) : len)

#include <cstring>
#include <cstdio>

namespace jstd 
{

#if defined(_WIN32)
   
    void check_errors() {
        DWORD lastErr = GetLastError();
        if (lastErr == ERROR_SUCCESS)
            return;
        if (lastErr == ERROR_ACCESS_DENIED)
            throw_except<sequrity_exception>(system::error_string(lastErr));
        else   
            throw_except<io_exception>(system::error_string(lastErr));
    }

    DWORD get_file_attrib(const char* path, int path_length) {
        tca::inline_linear_allocator<sizeof(wchar_t) * filesystem::MAX_LENGTH_PATH> allocator;
        u16string system_path = make_utf16(path, &allocator, byte_order::LE, path_length);
        return GetFileAttributesW(reinterpret_cast<const wchar_t*>(system_path.c_string()));
    }

    bool check_access(const char* path, int mode, int path_length){
        tca::inline_linear_allocator<sizeof(wchar_t) * filesystem::MAX_LENGTH_PATH> allocator;
        u16string system_path = make_utf16(path, &allocator, byte_order::LE, path_length);
        const wchar_t* npath = reinterpret_cast<const wchar_t*>(system_path.c_string());
		errno = 0;
		if (_waccess(npath, mode) != 0)
            return false;
		return true;
	}

    bool set_access(const char* path, int mode, bool on, int path_length) {
        tca::inline_linear_allocator<sizeof(wchar_t) * filesystem::MAX_LENGTH_PATH> allocator;
        u16string system_path = make_utf16(path, &allocator, byte_order::LE, path_length);
        const wchar_t* npath = reinterpret_cast<const wchar_t*>(system_path.c_string());

        struct _stat filestat;
        if (_wstat(npath, &filestat) != 0) {
            check_errors();
            return false;
        }

        mode_t newMode = filestat.st_mode;
        if (on){
            newMode |= mode;
        }else{
            newMode &= ~mode;
        }

        if (_wchmod(npath, newMode) != 0){
            check_errors();
            return false;
        }
        
        return true;
    }

    /*static*/ bool filesystem::exists(const char* path, int path_length) {
		DWORD attrib = get_file_attrib(path, path_length);
        if (attrib != INVALID_FILE_ATTRIBUTES)
            return true;
		return false;
    }

    /*static*/ bool filesystem::is_file(const char* path, int path_length) {
        if (!exists(path, path_length))
            return false;
        return !is_dir(path, path_length);
    }
    
    /*static*/ bool filesystem::is_dir(const char* path,  int path_length) {
		if (!exists(path, path_length))
            return false;
        DWORD attrib = get_file_attrib(path, path_length);
		if (attrib == INVALID_FILE_ATTRIBUTES){
			check_errors();
            return false;
		}
		return (attrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    /*static*/ int filesystem::runtime_path(char rbuf[], int bufsize) {
        wchar_t buf[MAX_LENGTH_PATH];
        DWORD len = GetCurrentDirectoryW(MAX_LENGTH_PATH, buf);
        if (len == 0)
            check_errors();
        tca::inline_linear_allocator<MAX_LENGTH_PATH * sizeof(utf16_t)> allocator;
        u8string path = make_utf8(reinterpret_cast<const uint16_t*>(buf), &allocator, byte_order::LE, len);
        int result_length = 0;
        int min_size = std::min(bufsize - 1, path.length());
        for (int i = 0; i < min_size; ++i, ++result_length)
            rbuf[i] = path.char_at(i);
        rbuf[result_length] = 0;
        return result_length;
    }

    /*static*/ bool filesystem::is_absolute(const char* path, int path_length) {
        path_length = normalize_length(path, path_length);
        if (path_length < 3)
            return false;
        char tom_name           = path[0];
        char double_dots        = path[1];
        char slash              = path[2];
        if (
            ((tom_name >= 'a' && tom_name <= 'z') || (tom_name >= 'A' && tom_name <= 'Z')) && 
            (double_dots == ':') && 
            (slash == '\\' || slash == '/')
        ) return true;
        return false;
    }

    /*static*/ uint64_t filesystem::length(const char* path, int path_length) {
        path_length = normalize_length(path, path_length);
        if (!is_file(path, path_length))
            return 0;

        tca::inline_linear_allocator<sizeof(wchar_t) * filesystem::MAX_LENGTH_PATH> allocator;
        u16string system_path = make_utf16(path, &allocator, byte_order::LE, path_length);

        HANDLE hFile = CreateFileW(
		    reinterpret_cast<const wchar_t*>(system_path.c_string()),               // имя файла
			GENERIC_READ,        		                                            // доступ для чтения
			FILE_SHARE_READ,     		                                            // разрешаем совместное чтение
			NULL,                		                                            // атрибуты безопасности
			OPEN_EXISTING,       		                                            // открываем существующий файл
			FILE_ATTRIBUTE_NORMAL, 		                                            // обычный файл
			NULL);               		                                            // шаблон файла

		if (hFile == INVALID_HANDLE_VALUE){
            check_errors();
            return 0;
        }
            
		LARGE_INTEGER fileSize;
		if (!GetFileSizeEx(hFile, &fileSize)) {
			CloseHandle(hFile);
			check_errors();
            return 0;
		}

		CloseHandle(hFile);
		return fileSize.QuadPart;
    }

    /*static*/ uint64_t filesystem::last_modified(const char* path, int path_length) {
        tca::inline_linear_allocator<sizeof(wchar_t) * filesystem::MAX_LENGTH_PATH> allocator;
        u16string system_path = make_utf16(path, &allocator, byte_order::LE, path_length);

		HANDLE hFile = CreateFileW(
        reinterpret_cast<const wchar_t*>(system_path.c_string()),
        FILE_WRITE_ATTRIBUTES,          // Доступ только к атрибутам
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,     // Необходимо для работы с директориями
        nullptr                         // шаблон файла
    );               		

		if (hFile == INVALID_HANDLE_VALUE) {
		    check_errors();
            return 0;
		}

		FILETIME lastModified;
		if (!GetFileTime(hFile, NULL, NULL, &lastModified)) {
		    CloseHandle(hFile);
		    check_errors();
            return 0;
		}
		CloseHandle(hFile);

		ULARGE_INTEGER time;

		time.LowPart    = lastModified.dwLowDateTime;
		time.HighPart   = lastModified.dwHighDateTime;

		return (time.QuadPart / 10000) - (uint64_t) 11644473600000;
    }

    /*static*/ bool filesystem::set_last_modified(const char* path, uint64_t ms_time, int path_length) {
        tca::inline_linear_allocator<sizeof(wchar_t) * filesystem::MAX_LENGTH_PATH> allocator;
        u16string system_path = make_utf16(path, &allocator, byte_order::LE, path_length);

        HANDLE hFile = CreateFileW(
            reinterpret_cast<const wchar_t*>(system_path.c_string()),
            FILE_WRITE_ATTRIBUTES,              // Доступ только для изменения атрибутов
            FILE_SHARE_READ | FILE_SHARE_WRITE, // Общий доступ
            NULL,                               // Защита по умолчанию
            OPEN_EXISTING,                      // Открыть только существующий файл
            FILE_FLAG_BACKUP_SEMANTICS,         // Обычный файл
            NULL                                // Без шаблона
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            check_errors();
            return false;
        }

        ULONGLONG fileTime  = (ms_time * 10000ULL) + 116444736000000000ULL; 
        FILETIME ft;
        ft.dwLowDateTime    = (DWORD) (fileTime);
        ft.dwHighDateTime   = (DWORD) (fileTime >> 32);
        BOOL result = SetFileTime(
            hFile,         // Хендл файла
            NULL,          // Время создания (не изменяем)
            NULL,          // Время последнего доступа (не изменяем)
            &ft            // Время последней модификации
        );
        CloseHandle(hFile); 
        return result; 
    }

    /*static*/ bool filesystem::create_new_file(const char* path, int path_length) {
        tca::inline_linear_allocator<sizeof(wchar_t) * filesystem::MAX_LENGTH_PATH> allocator;
        u16string system_path = make_utf16(path, &allocator, byte_order::LE, path_length);
        
        HANDLE file = CreateFileW(
            reinterpret_cast<const wchar_t*>(system_path.c_string()),           // Имя файла
            GENERIC_WRITE,                                                      // Режим доступа (запись)
            0,                                                                  // Совместный доступ (нет совместного доступа)
            NULL,                                                               // Атрибуты безопасности
            CREATE_NEW,                                                         // Создание нового файла, если он не существует
            FILE_ATTRIBUTE_NORMAL,                                              // Атрибуты файла (обычный файл)
            NULL                                                                // Шаблон файла (не используется)
        );

        if (file == INVALID_HANDLE_VALUE) {
            DWORD error = GetLastError();
            if (error == ERROR_FILE_EXISTS)
                return false;
            check_errors();
            return false;
        }
        
        CloseHandle(file);
        return true;
    }

    /*static*/ bool filesystem::mkdir(const char* path, int path_length) {
        tca::inline_linear_allocator<sizeof(wchar_t) * filesystem::MAX_LENGTH_PATH> allocator;
        u16string system_path = make_utf16(path, &allocator, byte_order::LE, path_length);
		
        bool created        = CreateDirectoryW(reinterpret_cast<const wchar_t*>(system_path.c_string()), null) != 0;
        bool alreadyExists  = GetLastError() == ERROR_ALREADY_EXISTS;
        
        if (created || alreadyExists){
            return true;
        }else{
            check_errors();
            return false;
        }
    }

    /*static*/ bool filesystem::remove(const char* path, int path_length) {
	    tca::inline_linear_allocator<sizeof(wchar_t) * filesystem::MAX_LENGTH_PATH> allocator;
        u16string system_path = make_utf16(path, &allocator, byte_order::LE, path_length);
		if (!DeleteFileW(reinterpret_cast<const wchar_t*>(system_path.c_string()))){
		    check_errors();
            return false;
		}
		return true;
    }

    /*static*/ bool filesystem::rename_to(const char* old_path, const char* new_path, int old_path_length, int new_path_length) {
        tca::inline_linear_allocator<sizeof(wchar_t) * filesystem::MAX_LENGTH_PATH * 2> allocator;
        u16string system_old_path = make_utf16(old_path, &allocator, byte_order::LE, old_path_length);
        u16string system_new_path = make_utf16(new_path, &allocator, byte_order::LE, new_path_length);
        if (_wrename(
            reinterpret_cast<const wchar_t*>(system_old_path.c_string()), 
            reinterpret_cast<const wchar_t*>(system_new_path.c_string())) != 0){
            if (errno == EACCES)
                check_errors();
            return false;
        }
        return true;
    }

    /*static*/ FILE* filesystem::open(const char* path, const char* mark, int path_length, int mark_length) {
        tca::inline_linear_allocator<(sizeof(wchar_t) * filesystem::MAX_LENGTH_PATH) + (sizeof(wchar_t) * 16)> allocator;
        u16string system_path = make_utf16(path, &allocator, byte_order::LE, path_length);
        u16string system_mark = make_utf16(mark, &allocator, byte_order::LE, mark_length);

        FILE* handle = _wfopen(
            reinterpret_cast<const wchar_t*>(system_path.c_string()), 
            reinterpret_cast<const wchar_t*>(system_mark.c_string())
        );   
        
        if (handle == nullptr) {
            if (errno == EACCES)
                throw_except<sequrity_exception>(strerror(errno));
            else
                throw_except<io_exception>(strerror(errno));
        }
        return handle;
    }

    bool is_current_or_prev_folder(const wchar_t* path) {
        int len = u16string::strlen(reinterpret_cast<const uint16_t*>(path));
        return (len == 1 && path[0] == '.') || (len == 2 && path[0] == '.' && path[1] == '.');
    }

    directory_iterator::directory_iterator(const char* path, int path_length) : _dir(), _entry(), _end(path == nullptr ? true : false) {
        if (path != nullptr) {
            tca::inline_linear_allocator<sizeof(wchar_t) * filesystem::MAX_LENGTH_PATH> allocator;
            u16string system_path = make_utf16(path, &allocator, byte_order::LE, path_length);
            {   //TODO: этот кусок говна нужен только потому, что винде нужен шаблон. 
                //Если просто передать "C:\\" -- то эта чёртова система не поймёт, что нужны все папки в указанной директории.
                uint16_t _template[] {filesystem::FILE_SEPARATOR, '*'};
                system_path.append(_template, 2, system::native_byte_order());
            }
            _dir = FindFirstFileW(reinterpret_cast<const wchar_t*>(system_path.c_string()), &_entry);
            if (_dir != INVALID_HANDLE_VALUE) {
                if (is_current_or_prev_folder(_entry.cFileName)) 
                    ++(*this);
            } else {
                _end = true;
            }
        }
    }   

    directory_iterator::directory_iterator(directory_iterator&& it) : _dir(std::move(it._dir)), _entry(std::move(it._entry)) {
        it._dir     = {0};
        //it._entry   = {0};
    }
    
    directory_iterator& directory_iterator::operator= (directory_iterator& it) {
        if (&it != this) {
            close();
            _dir    = std::move(it._dir);
            _entry  = std::move(it._entry);
            it._dir     = {0};
            //it._entry   = {0};
        } 
        return *this;
    }
    
    void directory_iterator::close() {
        if (_dir != INVALID_HANDLE_VALUE) {
            FindClose(_dir);
            _dir    = INVALID_HANDLE_VALUE;
            //_entry  = {0};
        }
    }

    directory_iterator::~directory_iterator() {
        close();
    }
    
    bool directory_iterator::operator!= (const directory_iterator& it) const {
        return _end != it._end;
    }
    
    directory_iterator& directory_iterator::operator++ () {   
        START:
        if (_end)
            throw_except<illegal_state_exception>("End of directory stream!");
        if (FindNextFileW(_dir, &_entry) != 0) {
            if (is_current_or_prev_folder(_entry.cFileName))
                goto START;
        } else {
            _end = true;
        }
        return *this;
    }
    
    directory_entry directory_iterator::operator* () const {
        tca::inline_linear_allocator<filesystem::MAX_LENGTH_PATH> allocator;
        const uint16_t* entry_path = reinterpret_cast<const uint16_t*>(_entry.cFileName);
        u8string path = make_utf8(entry_path, &allocator, byte_order::LE);
        return directory_entry(reinterpret_cast<const char*>(path.c_string()), path.length());
    }

#elif defined(__linux__)

    inline void copy_from_to(const char* in, int in_len, char buf[], int bufsize) {
        in_len          = normalize_length(in, in_len);
        int max_size    = std::min(bufsize - 1, in_len);
        std::memcpy(buf, in, in_len);
        buf[max_size] = 0;
    }

    void check_errors() {
        if (errno == 0)
            return;
        if (errno == EACCES)
            throw_except<sequrity_exception>(system::error_string(errno));
        else   
            throw_except<io_exception>(system::error_string(errno));
    }

    void fillStatStruct(const char* path, struct stat* filestat) {
        if (stat(path, filestat) != 0)
            check_errors();
    }

    bool check_access(const char* path, int mode, int path_length){
        char npath[filesystem::MAX_LENGTH_PATH];
        copy_from_to(path, path_length, npath, sizeof(npath));
		errno = 0;
		if (access(npath, mode) != 0)
            return false;
		return true;
	}

    bool set_access(const char* path, int mode, bool on, int path_length) {
        char npath[filesystem::MAX_LENGTH_PATH];
        copy_from_to(path, path_length, npath, sizeof(npath));

        struct stat filestat;
        fillStatStruct(npath, &filestat);

        mode_t newMode = filestat.st_mode;
        if (on){
            newMode |= mode;
        }else{
            newMode &= ~mode;
        }

        if (chmod(npath, newMode) != 0){
            check_errors();
            return false;
        }
        
        return true;
    }

    /*static*/ bool filesystem::exists(const char* path, int path_length){
		char npath[MAX_LENGTH_PATH];
        copy_from_to(path, path_length, npath, sizeof(npath));
        
        struct stat filestat;
		return stat(npath, &filestat) == 0;
	}

    /*static*/ bool filesystem::is_file(const char* path, int path_length){
        char npath[MAX_LENGTH_PATH];
        copy_from_to(path, path_length, npath, sizeof(npath));
        
        if (!exists(npath, path_length))
            return false;
        struct stat filestat;
        fillStatStruct(npath, &filestat);
        return S_ISREG(filestat.st_mode);
    }

    /*static*/ bool filesystem::is_dir(const char* path, int path_length){
        char npath[MAX_LENGTH_PATH];
        copy_from_to(path, path_length, npath, sizeof(npath));
        
        if (!exists(npath, path_length))
            return false;
        struct stat filestat;
        fillStatStruct(npath, &filestat);
        return S_ISDIR(filestat.st_mode);
    }

    /*static*/ int filesystem::runtime_path(char rbuf[], int bufsize) {
        int len = readlink("/proc/self/exe", rbuf, bufsize);
        if (len == 0)
            check_errors();
        for (int i = len - 1; i >= 0;--i) {
            if (rbuf[i] == '/') {
                rbuf[i] = '0';
                return i;
            }
        }
        return len;  
    }

    /*static*/ bool filesystem::is_absolute(const char* path, int path_length) {
        path_length = normalize_length(path, path_length);
        if (path_length < 1)
            return false;
        if (path[0] == '/' || path[1] == '\\')
            return true;
        return false;
    }

    /*static*/ uint64_t filesystem::length(const char* path, int path_length){
        if (!is_file(path, path_length))
            return 0;
        char npath[MAX_LENGTH_PATH];
        copy_from_to(path, path_length, npath, sizeof(npath));
        
        struct stat filestat;
		fillStatStruct(npath, &filestat);
		return filestat.st_size;
	}

    /*static*/ uint64_t filesystem::last_modified(const char* path, int path_length) {
        char npath[MAX_LENGTH_PATH];
        copy_from_to(path, path_length, npath, sizeof(npath));
        
        struct stat filestat;
        fillStatStruct(npath, &filestat);
		struct timespec& sTime = filestat.st_mtim;
        return (sTime.tv_nsec / 1000000) + (sTime.tv_sec * 1000);
    }

    /*static*/ bool filesystem::set_last_modified(const char* path, uint64_t ms_time, int path_length) {
        char npath[MAX_LENGTH_PATH];
        copy_from_to(path, path_length, npath, sizeof(npath));
        
        struct stat filestat;
        fillStatStruct(npath, &filestat);
        utimbuf timeBuf;
        timeBuf.actime  = filestat.st_atim.tv_sec;
        timeBuf.modtime = ms_time / 1000;  //ms to sec
        if (utime(npath, &timeBuf) != 0){
            check_errors();
            return false;
        }
        return true;
    }

    /*static*/ bool filesystem::create_new_file(const char* path, int path_length) {
        char npath[MAX_LENGTH_PATH];
        copy_from_to(path, path_length, npath, sizeof(npath));
        
        //       \/ это чёртово двойное двоеточие нужно, т.к иначе компилятор будет визжать, 
        //          из-за попытки вызвать filesystem::open(const char*, const char*, int, int).
        int fd = ::open(npath, O_CREAT, PERMISSION_ALL);
        if (fd == -1){
            if (errno == EEXIST)
                return false;
            check_errors();
            return false;
        }
        return true;
    }

    /*static*/ bool filesystem::mkdir(const char* path, int path_length) {
        char npath[MAX_LENGTH_PATH];
        copy_from_to(path, path_length, npath, sizeof(npath));
        
        if (::mkdir(npath, PERMISSION_ALL) != 0){
            check_errors();
            return false;
        }
        return true;
    }

    /*static*/ bool filesystem::remove(const char* path, int path_length) {
		char npath[MAX_LENGTH_PATH];
        copy_from_to(path, path_length, npath, sizeof(npath));
        
        if (::remove(npath) != 0){
			check_errors();
            return false;
		}
		return true;
    }

    /*static*/ bool filesystem::rename_to(const char* old_path, const char* new_path, int old_path_length, int new_path_length) {
        char opath[MAX_LENGTH_PATH];
        char npath[MAX_LENGTH_PATH];
        copy_from_to(old_path, old_path_length, opath, sizeof(opath));
        copy_from_to(new_path, new_path_length, npath, sizeof(npath));
		if (rename(opath, npath) != 0){
            if (errno == EACCES)
                check_errors();
            return false;
		}
		return true;
    }

    /*static*/ FILE* filesystem::open(const char* path, const char* mark, int path_length, int mark_length) {
        FILE* handle = fopen(path, mark);   
        if (handle == nullptr) {
            if (errno == EACCES)
                throw_except<sequrity_exception>(strerror(errno));
            else
                throw_except<io_exception>(strerror(errno));
        }
        return handle;
    }
    
    directory_iterator::directory_iterator(const char* path, int path_length) : _dir(nullptr), _entry(nullptr) {
        if (path != nullptr) {
            _dir = opendir(path);
            if (_dir == nullptr) {
                if (errno == EACCES) 
                    check_errors();
            } else {
                this->operator++();
            }
        }
    }
    
    directory_iterator::directory_iterator(directory_iterator&& it) : _dir(it._dir), _entry(it._entry) {
        it._dir     = nullptr;
        it._entry   = nullptr;
    }
    
    directory_iterator& directory_iterator::operator= (directory_iterator& it) {
        if (&it != this) {
            close();
            _dir        = it._dir;
            _entry      = it._entry;
            it._dir     = nullptr;
            it._entry   = nullptr;
        }
        return *this;
    }
    
    void directory_iterator::close() {
        if (_dir != nullptr) {
            closedir(_dir);
            _dir    = nullptr;
            _entry  = nullptr;
        }
    }

    directory_iterator::~directory_iterator() {
        close();
    }

    bool directory_iterator::operator!= (const directory_iterator& it) const {
        return _entry != it._entry;
    }
    
    directory_iterator& directory_iterator::operator++() {
        while (true) {
            _entry = readdir(_dir);
            if (_entry == nullptr)
                break;
            int len = std::strlen(_entry->d_name);
            if ((len == 1 && _entry->d_name[0] == '.') || (len == 2 && _entry->d_name[0] == '.' && _entry->d_name[1] == '.'))
                continue;
            break;
        }
        return *this;
    }
    
    directory_entry directory_iterator::operator*() const {
        return directory_entry(_entry->d_name);
    }

#endif

    /*static*/ void filesystem::close(FILE* handle) {
        if (fclose(handle) != 0)
            throw_except<io_exception>(strerror(errno));
    }

    /*static*/ void filesystem::flush(FILE* handle) {
        if (fflush(handle) != 0)
            throw_except<io_exception>(strerror(errno));
    }

    /*static*/ bool filesystem::is_separator(char ch) {
        return ch == '\\' || ch == '/';
    }

    /*static*/ bool filesystem::mkdirs(const char* path, int path_length) {
        //      /home/miracle/test
        //      C:/user/miracle/test
        int length = path_length < 0 ? std::strlen(path) : path_length;

        bool firstSeparator = false;
        for (int i = 0; i < length; ++i){
            char _ch = path[i];
            if (is_separator(_ch) && !firstSeparator){
                firstSeparator = true;
                continue;
            }
            if (is_separator(_ch) || i + 1 >= length){
                //char buf[1024];
                //memset(buf, 0, sizeof(buf));
                //memcpy(buf, path, i + 1);
                if (exists(path, i + 1))
                    continue;
                bool created = mkdir(path, i + 1);
                if (!created)
                    return false;
            }
        }
        return true;
    }

    /*static*/ bool filesystem::can_execute(const char* path, int path_length) {
        return check_access(path, CAN_EXECUTE, path_length);
    }
    
    /*static*/ bool filesystem::can_read(const char* path, int path_length) {
        return check_access(path, CAN_READ, path_length);
    }
    
    /*static*/ bool filesystem::can_write(const char* path, int path_length) {
        return check_access(path, CAN_WRITE, path_length);
    }

    /*static*/ bool filesystem::set_executable(const char* path, bool on_off, int path_length) {
        return set_access(path, CAN_EXECUTE, on_off, path_length);
    }

    /*static*/ bool filesystem::set_readable(const char* path, bool on_off, int path_length) {
        return set_access(path, CAN_READ, on_off, path_length);
    }
    
    /*static*/ bool filesystem::set_writable(const char* path, bool on_off, int path_length) {
        return set_access(path, CAN_WRITE, on_off, path_length);
    }

    /*static*/ int filesystem::count_files_in_directory(const char* path, file_filter* filter, int path_length) {
        directory_iterator begin(path, path_length);
        directory_iterator end;
        int count_files = 0;
        while (begin != end) {
            directory_entry entry = *begin;
            const char* name    = entry.get_name();
            int len             = std::strlen(name);    
            if (!filter->apply(name, len))
                goto NEXT;          // Чёрт возьми, чей этот дерьмовый код?
            ++count_files;
            NEXT:
            ++begin;
        }
        return count_files;
    }

    /*static*/ char const* const* filesystem::list_files(
        const char* path, tca::base_allocator* allocator, file_filter* filter, int path_length) {
        accept_all_filter accept_all;
        if (filter == nullptr)
            filter = &accept_all;
        int count_files = count_files_in_directory(path, filter, path_length);
        if (count_files == 0)
            return nullptr;
        char** array = (char**) allocator->allocate_align((sizeof(char*)) * (count_files + 1), alignof(char*));
        int off = 0;
        directory_iterator begin(path, path_length);
        directory_iterator end;
        while (begin != end) {
            directory_entry entry = *begin;
            const char* name    = entry.get_name();
            int len             = std::strlen(name);    
            if (!filter->apply(name, len))
                goto NEXT;
            {
                char* r_name = (char*) allocator->allocate(len + 1);
                std::memcpy(r_name, name, len);
                r_name[len] = '\0';
                array[off++] = r_name;
            }
            NEXT:
            ++begin;
        }
        array[count_files] = nullptr;
        return array;
    }

    /*static*/ void filesystem::free_list_files(char const* const* const files, tca::base_allocator* allocator) {
        if (files == nullptr)
            return;
        int i = 0;
        while(true) {
            const char* file_name = files[i++];
            if (file_name == nullptr)
                break;
            int len = std::strlen(file_name);
            allocator->deallocate(const_cast<char*>(file_name), len + 1);
        }
        allocator->deallocate(reinterpret_cast<void*>(const_cast<char**>(files)), sizeof(char*) * i);
    }

    /**
     * Заменяет двойной разделитель, на одинарный
     * /test///aa//bb//ccc" преобразутеся в /test/aa/bb/cc
     * 
     * Но если путь сетевой
     * //test///aa//bb//ccc" преобразутеся в //test/aa/bb/cc
     * 
     */
    int remove_double_separators(char path[], int path_length) {
        int wp = 0;
        int rp = 0;
        
        //проверка на то, что это сетевой путь. То есть, путь начинающийся на "\\" - "\\bla\bla\bla" 
        if (path_length >= 2 && filesystem::is_separator(path[rp]) && filesystem::is_separator(path[rp + 1]))
            rp += 2, wp += 2;

        while (rp < path_length) {    
            if ((filesystem::is_separator(path[rp]) && rp + 1 >= path_length) || (filesystem::is_separator(path[rp]) && (rp + 1 < path_length) && filesystem::is_separator(path[rp + 1]))) {
                ++rp;
                continue;
            } 
            path[wp++] = path[rp++];
        }
        return wp;
    }

    /**
     * Заменяет все разделители на константу зависящую от системы.
     * @see filesystem::FILE_SEPARATOR
     */
    void replace_separators_to_system_separators(char path[], int path_length) {
        for (int i = 0; i < path_length; ++i) {
            if (filesystem::is_separator(path[i]))
                path[i] = filesystem::FILE_SEPARATOR;
        }
    }

    /*static*/ int filesystem::normalize_path(char path[], int path_length) {
        path_length = remove_double_separators(path, normalize_length(path, path_length));
        int wp = 0;
        int rp = 0;
        while (rp < path_length) {
            
            if (rp != 0 && path[rp] == '.' && (rp + 1 < path_length) && is_separator(path[rp + 1])) {
                ++rp;
                if (wp > 0 && is_separator(path[wp - 1]))
                    --wp;
                continue;
            }
            //  C:/user/folder/../test
            else if (path[rp] == '.' && (rp + 2 < path_length) && path[rp + 1] == '.' && is_separator(path[rp + 2])) {
                rp += 3;
                if (wp > 0) {
                    --wp;
                    while (wp > 0 && !is_separator(path[wp - 1]))
                        --wp;
                }
                continue;
            }

            path[wp++] = path[rp++];
        }
        path[wp] = '\0';
        return wp;
    }

    int get_parent_from_absolute(char path[], int path_length = -1) {
        path_length = normalize_length(path, path_length);
        for (int i = path_length -1; i >= 0; --i) {
            if (filesystem::is_separator(path[i])) {
                path[i] = '\0';
                return i;
            }
        }
        return 0;
    }

    /*static*/ int filesystem::get_parent(const char* path, char out_path[], int buf_size, int path_length) {
        path_length = normalize_length(path, path_length);
        int min_size = std::min(buf_size - 1, path_length);
        std::memcpy(out_path, path, min_size);
        out_path[min_size] = '\0';
        path_length = filesystem::normalize_path(out_path, path_length);
        return get_parent_from_absolute(out_path, path_length);
    }



















































    directory_entry::directory_entry(const char* path, int path_length) {
        path_length     = normalize_length(path, path_length);
        int size        = std::min<int>(sizeof(_name) - 1, path_length);
        std::memcpy(_name, path, size);
        _name[size] = 0;
    }
    
    directory_entry::directory_entry(const directory_entry& ent) {
        std::memcpy(_name, ent._name, sizeof(_name));
    }
    
    directory_entry::directory_entry(directory_entry&& ent) {
        std::memcpy(_name, ent._name, sizeof(_name));
    }
    
    directory_entry& directory_entry::operator= (const directory_entry& ent) {
        if (&ent != this) {
            std::memcpy(_name, ent._name, sizeof(_name));
        }
        return *this;
    }
    
    directory_entry& directory_entry::operator= (directory_entry&& ent) {
        if (&ent != this) {
            std::memcpy(_name, ent._name, sizeof(_name));
        }
        return *this;
    }
    
    directory_entry::~directory_entry() {

    }
}