#include <allocators/linear_compact_allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <exception>
#include <iostream>

namespace tca {

    /**
     * 
     * ############################################################################
     *                           H E A D E R 
     * ############################################################################
     */
    Header::Header() : _reference(nullptr), _movfunc(nullptr), _size(0), _count(0) {}

    Header::Header(Header&& h) : _reference(h._reference), _movfunc(h._movfunc), _size(h._size), _count(h._count) {
        if (_reference != nullptr) {
            void* src = h.dataPointer();
            void* dst = dataPointer();
            _movfunc(dst, src, _count);
            _reference->_data = dst;
        }
        h._reference    = nullptr;
        h._movfunc      = nullptr;
        h._size         = 0;
        h._count        = 0;
    }
    
    Header::~Header() {
        if (_reference != nullptr) {
            _reference->_data = nullptr;
            _reference = nullptr;
        }
    }

    void* Header::dataPointer() {
        char* _this = reinterpret_cast<char*>(this);
        return _this + byteSize();
    }

    /**
     * 
     * ############################################################################
     *                           R E F 
     * ############################################################################
     */
    Ref::Ref() : _data(nullptr) {

    }
    
    Ref::Ref(void* data) : _data(data) {

    }
    
    Ref::Ref(Ref&& ref) : _data(ref._data) {
        ref._data = nullptr;
        updateRef();
    }
    
    Header* Ref::getHeader() {
        if (_data == nullptr)
            return nullptr;
        return reinterpret_cast<Header*>(reinterpret_cast<char*>(_data) - Header::byteSize());
    }

    const Header* Ref::getHeader() const {
        if (_data == nullptr)
            return nullptr;
        return reinterpret_cast<const Header*>(reinterpret_cast<char*>(_data) - Header::byteSize());
    }

    void Ref::updateRef() {
        Header* header = getHeader();
        if (header != nullptr) {
            header->_reference = this;
        }
    }

    void Ref::dispose() {
        if (_data != nullptr) {
            Header* header = getHeader();
            header->_reference = nullptr;
            _data = nullptr;
        }
    }

    Ref& Ref::operator= (Ref&& ref) {
        if (&ref != this) {
            dispose();
            _data       = ref._data;
            ref._data   = nullptr;
            updateRef();
        }
        return *this;
    }
    
    Ref::~Ref() {
        dispose();
    }

    /**
     * 
     * ############################################################################
     *                  L I N E A R _ C O M P A C T _ A L L O C A T O R
     * ############################################################################
     */

    compact_linear_allocator::compact_linear_allocator(base_allocator* allocator, std::size_t capacity) : _allocator(allocator), _capacity(capacity), _offset(0) {
        _data   = allocator->allocate_align(capacity, ALLOC_LIB_MAX_ALIGN);
        _tmp            = nullptr;
        _tmpCapacity    = 0;
    }

    compact_linear_allocator::compact_linear_allocator(compact_linear_allocator&& alloc) : 
    _allocator(alloc._allocator),
    _data(alloc._data),
    _capacity(alloc._capacity),
    _tmp(alloc._tmp),
    _tmpCapacity(alloc._tmpCapacity),
    _offset(alloc._offset)
    {
        alloc._allocator    = nullptr;
        alloc._data         = nullptr;
        alloc._capacity     = 0;
        alloc._tmp          = nullptr;
        alloc._tmpCapacity  = 0;
        alloc._offset       = 0;
    }
    
    compact_linear_allocator& compact_linear_allocator::operator= (compact_linear_allocator&& alloc) {
        if (&alloc != this) {
            free();
            _allocator      = alloc._allocator;
            _data           = alloc._data;
            _capacity       = alloc._capacity;
            _tmp            = alloc._tmp;
            _tmpCapacity    = alloc._tmpCapacity;
            _offset         = alloc._offset;

            alloc._allocator    = nullptr;
            alloc._data         = nullptr;
            alloc._capacity     = 0;
            alloc._tmp          = nullptr;
            alloc._tmpCapacity  = 0;
            alloc._offset       = 0;
        }
        return *this;
    }

    compact_linear_allocator::~compact_linear_allocator() {
        free();
    }

    void compact_linear_allocator::free() {
        if (_allocator != nullptr) {
            if (_tmp != nullptr)
                _allocator->deallocate(_tmp,    _tmpCapacity);
            if (_data != nullptr)
                _allocator->deallocate(_data,   _capacity);
            _allocator  = nullptr;
            _tmp        = nullptr;
            _data       = nullptr;
        }
    }

    Ref compact_linear_allocator::allocate(std::size_t sz, std::size_t count, void (*move_func)(void*, void*, std::size_t)) {
        std::size_t total = calcAlignSize(Header::byteSize() + sz, ALLOC_LIB_MAX_ALIGN);
        if (_offset + total > _capacity) {
            compact();
            if (_offset + total > _capacity) {
                std::size_t tmpCapacity = _capacity;

                //Попытка увеличить ёмкость, пока ёмкость не будет больше запрашиваемого блока памяти.
                while (tmpCapacity < (_capacity + total))
                    tmpCapacity <<= 1; //aka tmpCapacity *= 2
                grow(tmpCapacity);

                if (_offset + total > _capacity) 
                    throw jstd::make_except<jstd::out_of_memory_error>("Compact Allocator Out of Memory!");
            }
        }
        
        char* bytes = reinterpret_cast<char*>(_data);
        
        Header* header = new(bytes + _offset) Header();
        Ref ref(header->dataPointer());
    

        header->_size       = total;
        header->_movfunc    = move_func;
        header->_count      = count;
        header->_reference  = &ref;

        _offset += total;
        
        return Ref(std::move(ref));
    }

    void compact_linear_allocator::grow() {
        grow(_capacity << 1);
    }

    void compact_linear_allocator::grow(std::size_t newCapacity) {
        
        char* newData = reinterpret_cast<char*>(_allocator->allocate_align(newCapacity, ALLOC_LIB_MAX_ALIGN));
        
        for (std::size_t i = 0; i < _offset; ) {
            Header* src     = reinterpret_cast<Header*>(reinterpret_cast<char*>(_data) + i);
            Header* dst = new(newData + i) Header(std::move(*src));
            i += dst->_size;
        }

        _allocator->deallocate(_data, _capacity);
        _data       = newData;
        _capacity   = newCapacity;
    }
    
    void compact_linear_allocator::moveHeader(Header* dst, Header* src) {
        
        /**
         * Если размер объекта больше, чем буфер для временных объектов,
         * то происходит его увеличение.
         */
        {
            std::size_t sz = src->_size;
            if (_tmpCapacity < sz) {
                if (_tmp != nullptr)
                    _allocator->deallocate(_tmp, _tmpCapacity);
                _tmp            = _allocator->allocate_align(sz, ALLOC_LIB_MAX_ALIGN);
                _tmpCapacity    = sz;
            }
        }
        Header* tmp = new(_tmp) Header(std::move(*src)); 
        new (dst) Header(std::move(*tmp));
    }

    void compact_linear_allocator::compact() {
        std::size_t readPointer     = 0;
        std::size_t writePointer    = 0;
        char* const data = reinterpret_cast<char*>(_data);
        while (readPointer < _offset) {
            Header* srcHeader = reinterpret_cast<Header*>(data + readPointer);
            std::size_t size = srcHeader->_size;
            if (srcHeader->_reference == nullptr) {
                readPointer += size;
                continue;
            }

            Header* dstHeader = reinterpret_cast<Header*>(data + writePointer);
            
            if (dstHeader != srcHeader)
                moveHeader(dstHeader, srcHeader);

            writePointer    += size;
            readPointer     += size;
        }

        _offset = writePointer;
    }

    void compact_linear_allocator::print() const {
        std::printf("================== C O M P A C T   A L L O C A T O R ==================\n");
        std::printf("[capacity: %zu, tmpbuf %zu]\n", _capacity, _tmpCapacity);
        for (std::size_t i = 0; i < _offset; ) {
            Header* h = reinterpret_cast<Header*>(reinterpret_cast<char*>(_data) + i);
            std::printf("[size: %zu, 0x%llx]\n", h->_size, (long long) h->_reference);
            i += h->_size;
        }
    }
}