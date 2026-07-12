#include <cpp/lang/io/ibstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>
#include <cassert>

namespace jstd {

    ibstream::ibstream() : _allocator(nullptr), _buffer(nullptr), _capacity(0), _offset(0), _limit(0), _in(nullptr) {

    }

    ibstream::ibstream(istream* stream, tca::allocator* allocator, std::size_t buf_size) : ibstream() {
        char* data = (char*) allocator->allocate(buf_size);
        _allocator  = allocator;
        _buffer     = data;
        _capacity   = buf_size;
        _offset     = 0;
        _limit      = 0;
        _in         = stream;
    }
    
    ibstream::ibstream(istream* stream, char* buf, std::size_t buf_size) : ibstream() {
        _buffer     = buf;
        _capacity   = buf_size;
        _offset     = 0;
        _limit      = 0;
        _in         = stream;
    }
    
    ibstream::ibstream(ibstream&& stream) : 
    _allocator(stream._allocator), 
    _buffer(stream._buffer),
    _capacity(stream._capacity),
    _offset(stream._offset),
    _limit(stream._limit),
    _in(stream._in) {
        stream._allocator   = nullptr;
        stream._buffer      = nullptr;
        stream._capacity    = 0;
        stream._offset      = 0;
        stream._limit       = 0;
        stream._in          = nullptr;
    }
    
    ibstream& ibstream::operator= (ibstream&& stream) {
        if (&stream != this) {
            if (_in != nullptr)
                close();
            _allocator   = stream._allocator;
            _buffer      = stream._buffer;
            _capacity    = stream._capacity;
            _offset      = stream._offset;
            _limit       = stream._limit;
            _in          = stream._in;
            
            stream._allocator   = nullptr;
            stream._buffer      = nullptr;
            stream._capacity    = 0;
            stream._offset      = 0;
            stream._limit       = 0;
            stream._in          = nullptr;
        }
        return *this;
    }
    
    int ibstream::read() {
        return istream::read();
    }
    
    void ibstream::fill_buffer() {
        std::size_t readed = _in->read(_buffer, _capacity);
        _limit  = readed;
        _offset = 0;
    }

    /**
     * @return
     *      Count readed bytes.
     */
    std::size_t ibstream::read_from_buffer(char* v, std::size_t len) {
        assert(_limit >= _offset);
        std::size_t rem     = _limit - _offset;
        std::size_t readed  = rem < len ? rem : len;
        if (readed > 0)
        {
            memcpy(v, _buffer + _offset, readed);
            _offset += readed;
        }
        return readed;
    }

    std::size_t ibstream::read(char* buf, std::size_t sz) {
        assert(_limit >= _offset);
        std::size_t buf_rem = _limit - _offset;
        std::size_t readed  = 0;

        /**
         * 1. Попытаться прочитать данные из буфера.
         * 2. Если в буфере данных меньше, чем нужно:
         *      2.1 Читаем часть из буфера
         *      2.2 Если длина остаточной части больше, чем размер буфера, читаем сразу из файла, иначе читаем в буфер, а затем из буфера.
         */

        if (buf_rem >= sz)
        {
            readed = read_from_buffer(buf, sz);
        }
        else
        {
            std::size_t total_readed   = 0;
            std::size_t need_read      = sz;
            if (buf_rem > 0)
            {
                std::size_t readed = read_from_buffer(buf, need_read);
                total_readed    += readed;
                need_read       -= readed;
            }
            if (need_read > _capacity)
            {
                std::size_t readed = _in->read(buf + total_readed, need_read);
                total_readed += readed;
            }
            else
            {
                ibstream::fill_buffer();
                total_readed = read_from_buffer(buf + total_readed, need_read);
            }
            readed = total_readed;
        }
        
        return readed;
    }

    std::size_t ibstream::skip(std::size_t n) {
        JSTD_DEBUG_CODE(
            if (_in == nullptr)
                throw_except<io_exception>("Stream is null");
        );
        return istream::skip(n);
    }
    
    std::uintmax_t ibstream::available() const {
        assert(_limit >= _offset);
        std::size_t available_in_buffer = _limit - _offset;
        return available_in_buffer + _in->available();
    }
    
    void ibstream::free() {
        if (_allocator != nullptr)
        {
            _allocator->deallocate(_buffer, _capacity);
            _allocator  = nullptr;
        }
    }

    ibstream::~ibstream() {

    }

    void ibstream::close() {
        if (_in == nullptr)
            return;
        try
        {
            _in->close();
        } catch (const io_exception& e) {
            _in = nullptr;
            free();
            throw e;    
        }
        _in = nullptr;
        free();
    }    
}