#include <cpp/lang/compress/lz4_compressor.hpp>
#include <cpp/lang/math.hpp>
#include <utility>
#include <cassert>

namespace jstd
{
    
    lz4_compressor::lz4_compressor() : compressor(),
    rp(0),
    lit_start(0),
    state(state::FIND_MATCH),
    lit_len(0),
    lit_writed(0),
    offset(0),
    lit_rem(0),
    match_rem(0),
    match_len(0) {
        
    }

    lz4_compressor::lz4_compressor(lz4_compressor&& other) : compressor(std::move(other)),
    rp(0),
    lit_start(0),
    state(state::FIND_MATCH),
    lit_len(0),
    lit_writed(0),
    offset(0),
    lit_rem(0),
    match_rem(0),
    match_len(0) {
        std::swap(rp,           other.rp);
        std::swap(lit_start,    other.lit_start);
        std::swap(state,        other.state);
        std::swap(lit_len,      other.lit_len);
        std::swap(lit_writed,   other.lit_writed);
        std::swap(offset,       other.offset);
        std::swap(lit_rem,      other.lit_rem);
        std::swap(match_rem,    other.match_rem);
        std::swap(match_len,    other.match_len);
    }

    lz4_compressor& lz4_compressor::operator=(lz4_compressor&& other) {
        if (&other != this)
        {
            std::swap(rp,           other.rp);
            std::swap(lit_start,    other.lit_start);
            std::swap(state,        other.state);
            std::swap(lit_len,      other.lit_len);
            std::swap(lit_writed,   other.lit_writed);
            std::swap(offset,       other.offset);
            std::swap(lit_rem,      other.lit_rem);
            std::swap(match_rem,    other.match_rem);
            std::swap(match_len,    other.match_len);
        }
        return *this;
    }

    lz4_compressor::~lz4_compressor() {

    }
    
    lz4_compressor::u32 lz4_compressor::read_u32(const char* in) {
        return  ((u32) (unsigned char) in[0] << 0)   | 
                ((u32) (unsigned char) in[1] << 8)   | 
                ((u32) (unsigned char) in[2] << 16)  | 
                ((u32) (unsigned char) in[3] << 24);
    }

    /*static*/ std::size_t lz4_compressor::hash_idx(u32 val) {
        return (val * 0x85EBCA77u) & (TABLE_SIZE - 1);
    }

    void lz4_compressor::put_hash(u32 val, std::size_t pos) {
        hash_table[hash_idx(val)] = (u16) (pos & 0xffff);
    }

    std::size_t lz4_compressor::from_hash(u32 val) {
        std::size_t idx = hash_idx(val);
        return hash_table[idx] != 0xffffu ? hash_table[idx] : no_val;
    }

    void lz4_compressor::set_input(const char* in, std::size_t length) {
        compressor::set_input(in, length);
        state       = state::FIND_MATCH;
        for (std::size_t i = 0; i < TABLE_SIZE; ++i)
            hash_table[i] = 0xffffu;
    }

    void lz4_compressor::set_state(enum lz4_compressor::state state) {
        this->state = state;
        finish();
    }

    static std::size_t extend_match(const char* x, std::size_t current, std::size_t prev, std::size_t len) 
    {
        std::size_t matched = 0;
        std::size_t apos = current;
        std::size_t bpos = prev;
        while ((apos < len))
        {
            if (x[apos] != x[bpos])
                break;
            ++apos;
            ++bpos;
            ++matched;
        }        
        return matched;
    }

    void lz4_compressor::find_match() {
        assert(state == state::FIND_MATCH);
        const char* in = input;

        lit_start = rp;

        const std::size_t NO_COMPRESS_SIZE  = 5;
        const std::size_t TAIL              = NO_COMPRESS_SIZE + MATCH_LENGTH;

        while ((input_size > TAIL) && (rp < (input_size - TAIL)))
        {
            assert(rp < input_size);
            u32 seq             = read_u32(in + rp);
            std::size_t prev    = from_hash(seq);
            put_hash(seq, rp);

            //Эта последовательность встретилась впервые.
            if (prev == lz4_compressor::no_val)
            {
                ++rp;
                continue;
            }
            
            assert(rp >= prev);
            u16 dst = (u16)((u16)rp - (u16)prev);

            if (dst == 0)
            {
                ++rp;
                continue;   
            }
            
            //Проверяем, чтобы текущая последовательность совпадала с той, которая хранится в хеш-карте.
            if (read_u32(in + rp - dst) != seq)
            {
                ++rp;
                continue;
            }
            
            std::size_t prev_match_pos  = rp - dst;
            std::size_t match_len       = MATCH_LENGTH + extend_match(in, rp + MATCH_LENGTH, prev_match_pos + MATCH_LENGTH, input_size - NO_COMPRESS_SIZE);

            this->lit_len   = rp - lit_start;
            this->match_len = match_len;
            this->offset    = dst;

            rp += match_len;
            
            //Установить следующее состояние, как запись токена.
            set_state(state::WRITE_TOKEN);

            return;
        }

        //Дописать оставшиеся, не сжатые, данные
        if (rp < input_size)
        {
            this->lit_len   = input_size - lit_start;
            this->match_len = 0;
            this->offset    = 0;
            rp              = input_size;
            set_state(state::WRITE_TOKEN);
        }

    }
    
    std::size_t lz4_compressor::write_tok(char buf[], std::size_t sz) {
        if (sz == 0) return 0;
        
        char tok        = 0x00;
        char lit_tok    = 0;
        char match_tok  = 0;

        /*
            Если размер литерала или матча будет 15, то всё равно нужно будет прочитать нулевой байт, даже если rem будет равен нулю.
            При этом, если если длина литерала меньше 15, то rem даже не читается при записи

        */
        //длина литералов
        if (lit_len >= 15)
        {
            lit_tok = 15;
            lit_rem = lit_len - 15;
        }
        else
        {
            lit_tok |= (char) lit_len;
            lit_rem  = 0;
        }

        assert(match_len == 0 || match_len >= MATCH_LENGTH);
        std::size_t token_match = match_len == 0 ? 0 : match_len - MATCH_LENGTH;

        //длина мэтча
        if (token_match >= 15)
        {
            match_tok = 15;
            match_rem = token_match - 15;
        }
        else
        {
            match_tok |= (char) token_match;
            match_rem = 0;
        }
        
        //Если размер равен или больше 15, то нужно записать дополнительные байты, для кодирования длины.
        if (lit_len >= 15) set_state(state::WRITE_LIT_LEN_EXTRA);
        
        // Иначе просто записываем литералы.
        else                set_state(state::WRITE_LITERALS);

        *buf = (char) ((lit_tok << 4) | (match_tok << 0));

        return 1;
    }
    
    std::size_t lz4_compressor::write_lit_len_extra(char buf[], std::size_t sz) {
        if (sz == 0) return 0;
        std::size_t writed = 0;

        while ((writed < sz) && lit_rem >= 255)
        {
            buf[writed++] = (char) 255;
            lit_rem      -= 255;            
        }

        // Нужно записать остаток. Даже если 0, его всё равно нужно записать.
        if (writed < sz)
        {
            buf[writed++]   = (char) lit_rem;
            set_state(state::WRITE_LITERALS);
            lit_rem         = 0;
        }

        return writed;
    }
    
    std::size_t lz4_compressor::write_lit(char buf[], std::size_t sz) {
        if (sz == 0) return 0;

        assert(lit_len >= lit_writed);
        
        std::size_t write_rem   = lit_len - lit_writed;
        std::size_t writed      = math::min(sz, write_rem);
        std::memcpy(buf, input + lit_start + lit_writed, writed);
        lit_writed += writed;

        // Если записали все данные
        if (lit_writed == lit_len)
        {
            //Обнуляем для последующих записей.
            lit_writed = 0;
            
            // Если длина матча не нулевая, значит следом нужно записать смещение в little-endian порядке.
            if (match_len > 0)
            {
                //Начинаем с младшего слова.
                set_state(state::WRITE_OFFSET_L);
            }
            else
            {
                set_state(state::FIND_MATCH);
            }
        }

        return writed;
    }
    
    std::size_t lz4_compressor::write_offset(char buf[], std::size_t sz, offset_part part) {
        if (sz < 1) return 0;
        // std::printf("offset %zu\n", offset);
        switch (part) {
        case offset_part::LOW_WORD:
            *buf = (char) ((offset >> 0) & 0xFF);
            set_state(state::WRITE_OFFSET_H);
            break;
        case offset_part::HIGH_WORD:
            *buf = (char) ((offset >> 8) & 0xFF);
            //Если длина матча равна 15, то нужно записать дополнительные байты, иначе просто искать новый матч.
            assert(match_len >= MATCH_LENGTH);
            std::size_t token_match = match_len - MATCH_LENGTH;
            if (token_match >= 15)
                set_state(state::WRITE_MATCH_LEN_EXTRA);
            else
                set_state(state::FIND_MATCH);
            break;
        }
        return 1;
    }
    
    std::size_t lz4_compressor::write_match_len_extra(char buf[], std::size_t sz) {
        if (sz == 0) return 0;
        std::size_t writed = 0;

        while ((writed < sz) && match_rem >= 255)
        {
            buf[writed++] = (char) 255;
            match_rem    -= 255;
        }

        // Нужно записать остаток. Даже если 0, его всё равно нужно записать.
        if (writed < sz)
        {
            buf[writed++]   = (char) match_rem;
            set_state(state::FIND_MATCH);
            match_rem         = 0;
        }

        return writed;
    }

    bool lz4_compressor::finished() const {
        return rp >= input_size;
    }

    std::size_t lz4_compressor::compress(char out[], std::size_t outlen) {

        std::size_t writed  = 0;
        std::size_t rem     = outlen;

        while ((rem > 0) && (state != state::FIHISHED))
        {            
            std::size_t current_writed = 0;
            switch (state) {
                case state::FIND_MATCH:
                    if (finished())
                        set_state(state::FIHISHED);
                    else
                        find_match();
                    break;
                case state::WRITE_TOKEN:
                    current_writed = write_tok(out, rem);
                    break;
                case state::WRITE_LIT_LEN_EXTRA:
                    current_writed = write_lit_len_extra(out, rem);
                    break;
                case state::WRITE_LITERALS:
                    current_writed = write_lit(out, rem);
                    break;
                case state::WRITE_OFFSET_L:
                    current_writed = write_offset(out, rem, offset_part::LOW_WORD);
                    break;
                case state::WRITE_OFFSET_H:
                    current_writed = write_offset(out, rem, offset_part::HIGH_WORD);
                    break;
                case state::WRITE_MATCH_LEN_EXTRA:
                    current_writed = write_match_len_extra(out, rem);
                    break;
                default:
                    set_state(state::FIHISHED);
                    break;
            }
            //Увеличиваем коливество всех записей
            writed  += current_writed;
            //Двигаем указатель записи
            out     += current_writed;
            //Уменьшаем количество свободного места в выходном буфере
            assert(rem >= current_writed);
            rem     -= current_writed;
        }

        return writed;
    }
}

#if 0

#include <lz4.h>

#include <cstdio>

#if 0
    int main() {
        const char* str = "dskf.siofj0ffj23fnm,xdnfsdfh8w3fsiofj0ffj23fnm,xdnfsdfh8w3ffj0ffj23fnm,xdnfsdf";
        char buf[1024];
        std::size_t len = (std::size_t) LZ4_compress_default(str, buf, (int) std::strlen(str), sizeof(buf));
        for (std::size_t i = 0; i < len; ++i)
        {
            std::printf("%x ", (char) buf[i]);
        }
        // 0x5c
    }
#elif 0
    // const char* str = "Однако официальный стандарт LZ4 рекомендует делать эту границу чуть шире (то самое правило 12 байт до конца блока для поиска совпадений: input_len - 12), чтобы компрессор даже не тратил время на чтение хэшей там, где совпадение физически не сможет составить минимальные 4 байта и при этом оставить 5 байт литералов. Но для корректности вывода вашего текущего теста — ваша логика идеальна";
    const char* str = "abraabraabraabraabraabraabraabraabraabraabraabraabraabraabraabraabraabraabraabraabraabraabraabra";
    int main() {
        const std::size_t BUF_SIZE = 1024;
        char buf0[BUF_SIZE];
        char buf1[BUF_SIZE];

        std::size_t len0, len1;

        {
            len0 = (std::size_t) LZ4_compress_default(str, buf0, (int) std::strlen(str), BUF_SIZE);
            for (std::size_t i = 0; i < len0; ++i)
            {
                std::printf("%x ", (char) buf0[i]);
            }
        }
        std::printf("\n");
        {
            tc::lz4_compressor compressor;
            compressor.set_input(reinterpret_cast<const char*>(str), std::strlen(str));            
            len1 = compressor.compress((char*) buf1, BUF_SIZE);
            for (std::size_t i = 0; i < len1; ++i)
            {
                std::printf("%x ", (char) buf1[i]);
            }
        }
    }
#elif 0
    //const char* str = "A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z2A3B4C5D6E7F8G9H0I1J2K3L4M5N6O7P8Q9R0S1T2U3V4W5X6Y7Z8A9B0C1D2E3F4G5H6I7J8K9L0M1N2O3P4Q5R6S7T8U9V0W1X2Y3Z4A5B6C7D8E9F0G1H2I3J4K5L6M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6A7B8C9D0E1F2G3H4I5J6K7L8M9N0O1P2Q3R4S5T6U7V8W9X0Y1Z";
    //const char* str = "Многие вики позволяют изменять своё содержимое всем желающим, а не только зарегистрированным пользователям. Подобно тому, как стены зданий и заборы исписывают непристойными надписями и украшают рисунками граффити, в таких вики иногда портят содержимое или добавляют что-то неуместное. Но, в отличие от стен и заборов, в вики легко вернуть содержимое к ранней версии: исправлять легче, чем портить. Если же кто-либо настойчиво и намеренно стремится навредить пользователям вики-сайта, можно закрыть ему возможность вносить правки.";
    const char* str = "абракадабраабракадабраабракадабраабракадабраабракадабраабракадабраабракадабраабракадабраабракадабраабракадабра";
    int main() {
        const std::size_t BUF_SIZE = 1 << 15;
        char buf0[BUF_SIZE];
        char buf1[BUF_SIZE];

        std::size_t len0, len1;

        // {
        //     len0 = (std::size_t) LZ4_compress_default(str, buf0, (int) std::strlen(str), BUF_SIZE);
        //     for (std::size_t i = 0; i < len0; ++i)
        //     {
        //         std::printf("%x ", (char) buf0[i]);
        //     }
        // }
        
        // std::printf("\n");
        
        {
            tc::lz4_compressor compressor;
            compressor.set_input(reinterpret_cast<const char*>(str), std::strlen(str));            
            len1 = compressor.compress((char*) buf1, BUF_SIZE);
            // len1 = LZ4_compress_default(str, buf1, std::strlen(str), BUF_SIZE);

            std::printf("length: %zu\n", len1);

            for (std::size_t i = 0; i < len1; ++i)
            {
                std::printf("%x ", (unsigned char) buf1[i]);
            }

            std::printf("%\n");

            char buf[BUF_SIZE];
            int decompressed = LZ4_decompress_safe(buf1, buf, len1, sizeof(buf));
            for (int i = 0; i < decompressed; ++i)
            {
                std::printf("%c", (unsigned char) buf[i]);
            }

        }

    }
#else
    int main() {
         const char* str = "a0B2c4D6f8H1j3L5n7P9r1T3v5X7z9A2C4E6G8I0K2M4O6Q8S0U2W4Y6Z8b1d3f5h7j9l1n3p5r7t9v1x3z5!7@9#2$4%6^8&0*1(2)3-4=5+6[7]8{9}0|1:2;3,4.5<6>7?8~9`0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6a7b8c9d0e1f2g3h4i5j6k7l8m9n0o1p2q3r4s5t6u7v8w9x0y1z2@3#4$5%6^7&8*9(0)1-2=3+4[5]6{7}8|9:0;1<2>3?4~5`6";
        // const char* str = "a0B2c4D6f8H1j3L5n7P9r1T3v5X7z9A2C4E6G8I0K2M4O6Q8S0U2W4Y6Z8b1d3f5h7j9l1n3p5r7t9v1x3z5!7@9#2$4%6^8&0*1(2)3-4=5+6[7]8{9}0|1:2;3,4.5<6>7?8~9`0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S9T0U1V2W3X4Y5Z6a7b8c9d0e1f2g3h4i5j6k7l8m9n0o1p2q3r4s5t6u7v8w9x0y1z2@3#4$5%6^7&8*9(0)1-2=3+4[5]6{7}8|9:0;1<2>3?4~5`6";
        //const char* str = "dskf.siofj0ffj23fnm,xdnfsdfh8w3fsiofj0ffj23fnm,xdnfsdfh8w3ffj0ffj23fnm,xdnfsdf";
        //const char* str = "абракадабрабраадакадабра";
        char buf[1024];
        
        try {
            tc::lz4_compressor compressor;
            compressor.set_input(reinterpret_cast<const char*>(str), std::strlen(str));
            
            std::size_t len = compressor.compress(buf, sizeof(buf));
            std::printf("!writed: %zu\n", len);
            
            (0x61 + 0xf + 4);

            for (std::size_t i = 0; i < len; ++i)
            {
                std::printf("%x ", buf[i]);
            }
            
            const std::size_t v = 0x61 + 0xf + 4;
            
            // std::printf("\n");
            // for (std::size_t i = 0; i < len; ++i)
            // {
            //     std::printf("%d ", (unsigned int) buf[i]);
            // }

            // std::FILE* f = std::fopen("text.bin", "wb");
            // std::fwrite(buf, 1, len, f);
            // std::fclose(f);

            //std::printf("len: %zu, compressed: %zu\n", std::strlen(str), len);

            // for (std::size_t i = 0; i < len; ++i)
            // {
            //     std::printf("%c", buf[i]);
            // }
            // std::printf("\n");
        } catch (const tc::throwable& e) {
            std::printf("%s\n", e.cause());
        }
    }
#endif



// #include <cpp/lang/utils/array_list.hpp>



// int main() {
//     tc::array_list<int> list;
//     list.add(1);
//     list.add(-6);
//     list.add(2);
//     list.add(4);
//     list.add(6);
//     list.add(234);
//     list.add(2);

//     tc::utils::quick_sort(list.data(), list.size());

//     for (const int& a : list)
//     {
//         printf("%d\n", a);
//     }

//     list.clear();

//     std::printf("search start\n");
//     std::size_t idx = list.binary_search(234);
//     std::printf("search: %zu\n", idx);

// }

#endif

#if 1

#include <cstdio>
#include <iostream>
#include <cpp/lang/compress/lz4_compressor.hpp>
#include <cpp/lang/compress/lz4_decompressor.hpp>
#include <cpp/lang/io/omstream.hpp>
#include <cpp/lang/io/imstream.hpp>
#include <cpp/lang/io/odstream.hpp>
#include <cpp/lang/io/idstream.hpp>

int main() {
    tc::omstream mem(1024);
    tc::odstream ds(&mem);

    for (std::size_t i = 0; i < 10; ++i)
    {
        ds.write<float>(3.141f);
        ds.write<int>(242947);
        ds.write<long long>(7754345436);
    }

    std::printf("stream_size: %zu\n", mem.offset());


    char compressed[1024];
    std::size_t compressed_size;
    {
        tc::lz4_compressor compressor;
        compressor.set_input(mem.data(), mem.offset());
        compressed_size = compressor.compress(compressed, sizeof(compressed));
        std::printf("compressed: %zu\n", compressed_size);
    }

    char decompressed[1024];
    std::size_t decompressed_size;
    {
        tc::lz4_decompressor decompressor;
        decompressor.set_input(compressed, compressed_size);
        decompressed_size = decompressor.decompress(decompressed, sizeof(decompressed));
    }
    
    tc::imstream im(decompressed, decompressed_size);
    tc::idstream id(&im);
        for (std::size_t i = 0; i < 10; ++i)
    {
        std::cout << id.read<float>() << std::endl;
        std::cout << id.read<int>() << std::endl;
        std::cout << id.read<long long>() << std::endl;
    }
}

#endif