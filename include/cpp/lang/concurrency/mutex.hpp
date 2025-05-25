#ifndef JSTD_CPP_LANG_CONCURRENCY_H
#define JSTD_CPP_LANG_CONCURRENCY_H

#include <mutex>

namespace jstd
{

class mutex {
    mutex(const mutex&)             = delete;
    mutex& operator= (const mutex&) = delete;
    mutex(mutex&&)                  = delete;
    mutex& operator= (mutex&&)      = delete;
    std::mutex m_mutex_impl;
public:
    mutex();
    ~mutex();
    void lock();
    void unlock();
    bool try_lock();
};

}// namespace jstd

#endif//JSTD_CPP_LANG_CONCURRENCY_H