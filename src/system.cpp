#include <cpp/lang/system.hpp>
#include <cpp/lang/concurrency/mutex.hpp>
#include <cstdarg>
#include <cstdio>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__)
    #include <cstring>
    #include<sys/time.h>
	#include<time.h>
#endif

namespace jstd::system {   

    byte_order init_native_byte_order() {
        const unsigned long c   = 1;
        const unsigned char* ip = reinterpret_cast<const unsigned char*>(&c);
        return *ip != 0 ? byte_order::LE : byte_order::BE;
    }
    
    byte_order native_byte_order() {
        static const byte_order system_order = init_native_byte_order();
        return system_order;
    }

    int64_t current_time_millis() {
#if defined(__linux__) || defined(__APPLE__)
        timeval time;
        gettimeofday(&time, NULL);
        return (int64_t) (time.tv_sec * 1000) + (time.tv_usec / 1000);
#elif _WIN32
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        ULARGE_INTEGER time;
        time.LowPart    = ft.dwLowDateTime;
        time.HighPart   = ft.dwHighDateTime;
        ULONGLONG millesec = time.QuadPart / 10000ULL;
        return (int64_t) (millesec - 11644473600000ULL);
#endif
    }
    
    int64_t current_time_seconds() {
        return current_time_millis() / 1000;
    }

    int64_t nano_time() {
		const unsigned long NS_SECOND = 1000000000;
#if defined(__linux__) || defined(__APPLE__)
			timespec time;
			clock_gettime(CLOCK_MONOTONIC, &time);
			return (int64_t) (time.tv_sec * NS_SECOND) + time.tv_nsec;
#elif _WIN32
			LARGE_INTEGER freeq, counter;
			QueryPerformanceFrequency(&freeq);
			QueryPerformanceCounter(&counter);
			return (int64_t) (((double)counter.QuadPart / (double)freeq.QuadPart) * NS_SECOND);
#endif
    }


#if defined(_WIN32)
    const char* error_string(int err) {
        switch(err){
			case ERROR_SUCCESS              : return "No error";
			case ERROR_INSUFFICIENT_BUFFER  : return "The buffer size is insufficient to store the full path to the file";
			case ERROR_INVALID_HANDLE       : return "Invalid handle";
			case ERROR_MOD_NOT_FOUND        : return "Module not found";
			case ERROR_ACCESS_DENIED        : return "Access denied";
			case ERROR_INVALID_PARAMETER    : return "Invalid parameter";
			case ERROR_NOT_ENOUGH_MEMORY    : return "There is not enough memory to perform the operation";
			case ERROR_INVALID_FUNCTION     : return "The wrong was called to perform the operation";
			case ERROR_BAD_PATHNAME         : return "Bad path name";
			case ERROR_FILE_NOT_FOUND       : return "File not found";
			case ERROR_INVALID_NAME         : return "Invalid name";
			case ERROR_BAD_FORMAT           : return "Bad format";
            case ERROR_NOACCESS             : return "Invalid access to memory location";
			default:                          return "No specified error";
		}
    }
#elif defined(__linux__)
    const char* error_string(int err) {
        return strerror(err);
    }
#endif

    int tsprintf(const char* format, ...) {
        static jstd::mutex ls_mutex;
        ls_mutex.lock();
            std::va_list args;
            va_start(args, format);
            int result = std::vprintf(format, args);
            va_end(args);
        ls_mutex.unlock();
        return result;
    }
}