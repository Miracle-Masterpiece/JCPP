#ifndef JSTD_CPP_LANG_UTILS_COND_COMPILATION_H
#define JSTD_CPP_LANG_UTILS_COND_COMPILATION_H

#if !defined(NDEBUG)
#   define JSTD_DEBUG_CODE(code) code
#else
#   define JSTD_DEBUG_CODE(code)
#endif

#if defined(__linux__) || defined(linux)
#   define JSTD_OS_LINUX
#elif defined(_unix_)
#   define JSTD_OS_UNIX
#elif defined(_WIN32) || defined(WIN32)
#   define JSTD_OS_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
#   define JSTD_OS_MAC
#endif

#endif//JSTD_CPP_LANG_UTILS_COND_COMPILATION_H