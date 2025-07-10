#include <iostream>
#include <cpp/lang/exceptions.hpp>


void _throw() {
    jstd::throw_except<jstd::file_not_found_exception>("Файл не найден!");
}

void _close() {
    try {
        jstd::throw_except<jstd::out_of_memory_error>("Нет памяти!");
    } catch (const jstd::throwable& t) {
        //ignore
    }
}

void _do() {
    try {
        _throw();
    } catch (...) {
        _close();
        throw;
    }
}

#include <cpp/lang/io/ifstream.hpp>
#include <cpp/lang/io/ofstream.hpp>
#include <cpp/lang/logging/logger.hpp>

int main() {
    


    using namespace jstd::log;

    jstd::ofstream out(jstd::file("./session.log"), true);

    logger log = &out;
    
    char c;
    while(true) {
        std::cout << "next: ";
        std::cin >> c;
        if (c == 'e')
            break;
        log.info("bla-bla-bla");
        log.warn("bla-bla-bla");
        log.error("bla-bla-bla");
        try {
            jstd::ifstream in(jstd::file("fdsfsd"));
        } catch (jstd::io_exception& e) {
            log.except(e);
        }
    }
    
    out.flush();
    out.close();
}