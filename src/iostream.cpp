#include <cpp/lang/io/istream.hpp>
#include <cpp/lang/io/ostream.hpp>

namespace jstd {
    
    int64_t istream::skip(int64_t n) {
        int64_t skipped = 0;
        while (n-- > 0) {
            int readed = read();
            if (readed == -1)
                break;
            ++skipped;
        }
        return skipped;
    }

    istream::~istream() {

    }

    ostream::~ostream() {
        
    }

    void ostream::write(char c) {
        write(&c, 1);
    }

    int istream::read() {
        char c;
        int64_t readed = read(&c, 1);
        if (readed == -1)
            return -1;
        return c & 0xFF;
    }
} 