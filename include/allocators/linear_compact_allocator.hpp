#ifndef _ALLOCATORS_LINEAR_COMPACT_ALLOCATOR_ALLOCATOR_H
#define _ALLOCATORS_LINEAR_COMPACT_ALLOCATOR_ALLOCATOR_H

#include <allocators/base_allocator.hpp>
#include <allocators/Helpers.hpp>

#include <cstring>
#include <cstddef>
#include <utility>
#include <cstdio>
#include <cstdint>

#ifndef ALLOC_LIB_MAX_ALIGN
#define ALLOC_LIB_MAX_ALIGN alignof(std::max_align_t)
#endif

#ifndef NDEBUG
    #include <cassert>
    #define ALLOC_LIB_CHECK_ALIGN(ptr, type) assert( ( ((std::intptr_t) ptr) % alignof(type) ) == 0 )
    #define ALLOC_LIB_CHECK_NULL_POINTER(ptr) assert(ptr)
    #define ALLOC_LIB_CHECK_INDEX(idx, len) assert(idx < len);
#else
    #define ALLOC_LIB_CHECK_ALIGN(ptr, type)
    #define ALLOC_LIB_CHECK_NULL_POINTER(ptr)
    #define ALLOC_LIB_CHECK_INDEX(idx, len)
#endif

namespace tca {

    struct Ref;
    struct Header {
        
        //указатель на владеющий объект tca::Ref
        //Если этот указатель равен nullptr - значит что блок свободный.
        Ref* _reference;
        
        /**
         * Указатель на функцию, которая занимается перемещением данных из src в dst
         * 
         * @param dst
         *          Указатель на начало данных, откуда будет совершаться перемещение.
         * 
         * @param src
         *          Указатель на начало данных, куда будет совершаться перемещение.
         * 
         * @param count
         *          Количество объектов, выделенных аллокатором.
         *          Используется, если блок памяти аллоцировался, как массив.
         */
        void (*_movfunc) (void* dst, void* src, std::size_t count);
        
        //Общий размер выделенного блока, включая заголовок(Этот объект).
        std::size_t _size;  //size_with_header
        
        //Количество непрерывно выделенных объектов.
        std::size_t _count;
    
        /**
         * Создаёт объект заголовка по-умолчанию
         * и всем полям присваивает null-значение.
         */
        Header();
        
        /**
         * Перемещает данные заголовка в текущий объект
         * и, если _reference != nullptr обновляет указатель на tca::Header на this.
         */
        Header(Header&&);    
        
        /**
         * Если _reference != nullptr, то в tca::Ref::_data устанавливается значение null.
         */
        ~Header();
        
        /**
         * Возвращает указатель на блок памяти, которые следуют после заголовка.
         *
         * Структура такая:
         * [header... padding][data... padding]
         * padding - неиспользуемые байты, которые нужны для выравнивания стурктур.
         * 
         * 
         * Implementation defined:
         * Указатель на данные высчитывается как ((char*) this) + tca::Header::byteSize()
         */
        void* dataPointer();
    
        /**
         * Возвращает выровненный размер структуры так, 
         * чтобы размер был кратен максимальному выравниванию.
         * 
         * @remark
         *          Максимальное выравнивание устанавливается с помощью макроса ALLOC_LIB_MAX_ALIGN
         *          И по-умолчанию равно alignof(std::max_align_t)
         */
        constexpr static std::size_t byteSize() {
            return calcAlignSize(sizeof(Header), ALLOC_LIB_MAX_ALIGN);
        }
    
    private:
        /**
         * Удалённые методы копирования.
         */
        Header(const Header&) = delete;
        Header& operator= (const Header&)  = delete;
        
        //Так же удалён, поскольку перемещение обязано быть всегда с помощью конструктора перемещения и рамещением объекта через placement-new
        Header& operator= (Header&&) = delete;
    };
    
    /**
     * Стуктура для хранения указателя на владеющий блок памяти.
     */
    struct Ref {
        
        //указатель на начало выделенного блока.
        void* _data;
       
        /**
         * Создаёт объект, представляющий null-pointer
         */
        Ref();
    
        /**
         * Создаёт объект принимая указатель на начало блока, 
         * которым будет владеть этот объект.
         * 
         * @param data
         *          Указатель на блок памяти, которым будет владеть этот объект.
         * 
         * @remark
         *          Перед блоком памяти обязан располагаться объект tca:Header,
         *          для правильной работы tca::Ref::getHeader()
         */
        Ref(void* data);
        
        /**
         * Конструктор перемещения.
         * Перемещает ссылку из одного объекта в другой.
         * И, если новый указатель _data != nullptr, 
         * переходит в заголовок и меняет tca::Header::_ref на указатель текущего объекта.
         */
        Ref(Ref&&);
        
        /**
         * Оператор перемещения.
         * 
         * 
         * Перемещает ссылку из одного объекта в другой.
         * И, если новый указатель _data != nullptr, 
         * переходит в заголовок и меняет tca::Header::_ref на указатель текущего объекта.
         */
        Ref& operator= (Ref&&);
        
        /**
         * Помечает блок памяти, как свободный.
         * 
         * @see tca::Ref::free()
         */
        ~Ref();
        
        /**
         * Возвращает указатель на заголовок выделенного блока.
         * 
         * @return 
         *     Указатель на заголовок выделенного блока.
         * 
         * @see tca::Header
         */
        Header* getHeader();
        
        /**
         * Возвращает константный указатель на заголовок выделенного блока.
         * 
         * @return 
         *     Константный указатель на заголовок выделенного блока.
         * 
         * @see tca::Header
         */
        const Header* getHeader() const;
    
    private:
        //обновляет ссылки при перемещении
        void updateRef();
        
        /**
         * Помечает блок памяти, как свободный.
         */
        void dispose();
        
        //Удалённые конструктор копирования и оператор копирования.
        Ref(const Ref&);
        Ref& operator= (const Ref&);
    };
    
    /**
     * Структура для хранения внутренней безтиповой ссылки для интерпретирования её, как типовой.
     * Структура использует принципы std::unique_ptr<T>
     * То есть, её нельзя копировать, только перемещать и владеет блоком памяти только один объект структуры.
     * 
     * Структура имеет перегруженные операторы для работы с блоком памяти, как с прямым указателем.
     * При этом её можно привести к сырому указателю, чтобы, например, передать в функции, которые ожидают сырой указатель.
     * 
     */
    template<typename T>    
    struct reference {
    private:
        //удалённые конструкторы копирования и оператор присваивания с копированием.
        reference(const reference&);
        reference& operator= (const reference&);
    
        //Безтиповая внутренняя ссылка
        Ref _ref;
    
    public:
        /**
         * Создаёт объект nullreference
         */
        reference() : _ref() {
    
        }
    
        /**
         * Создаёт объект на основе безтиповой ссылки и
         * просто вызывает конструтор перемещения tca::Ref
         * 
         * @param ref
         *      R-value cсылка на объект 
         *      хранящий безтиповой указатель на блок памяти.      
         *      
         * 
         * 
         * @see tca::Ref
         */
        reference(Ref&& ref) : _ref(std::move(ref)) {
    
        }
    
        /**
         * Просто вызывает конструтор перемещения tca::Ref
         * 
         * @param ref
         *      R-value ссылка на tca::reference<T> 
         * 
         * @see tca::Ref
         */
        reference(reference&& ref) : _ref(std::move(ref._ref)) {
    
        }
    
        /**
         * Оператор перемещения.
         * Просто вызывает оператор перемещения у tca::Ref
         * 
         * @see tca::Ref
         */
        reference& operator= (reference&& ref) {
            if (&ref != this)
                _ref = std::move(ref._ref);
            return *this;
        }
    
        /**
         * Оператор приведения типа.
         * 
         * @remark
         *      Указатель, который возвращается, необходимо проверять на nullptr
         *      поскольку функция не бросает исключения, или std::abort(), если внутренний указатель nullptr
         * 
         * @return 
         *      Сырой указатель на T         
         */
        operator T*() {
            return reinterpret_cast<T*>(_ref._data);    
        } 
    
        /**
         * Оператор приведения типа.
         * 
         * @remark
         *      Указатель, который возвращается, необходимо проверять на nullptr
         *      поскольку функция не бросает исключения, или std::abort(), если внутренний указатель nullptr
         * 
         * @return 
         *      Сырой указатель на const T         
         */
        operator const T*() const {
            return reinterpret_cast<const T*>(_ref._data);    
        } 
    
        /**
         * Оператор доступа к членам.
         * 
         * @remark
         *      Если не объявлен макрос NDEBUG, 
         *      то функция сначала проверит внутренний указатель на nullptr.
         *      И если указатель равен nullptr, 
         *      то аварийно завершит программу с информацией об ошибке.
         */
        T* operator-> () {
            ALLOC_LIB_CHECK_NULL_POINTER(_ref._data);
            return reinterpret_cast<T*>(_ref._data);
        }
    
        /**
         * Константный оператор доступа к членам.
         * 
         * @remark
         *      Если не объявлен макрос NDEBUG, 
         *      то функция сначала проверит внутренний указатель на nullptr.
         *      И если указатель равен nullptr, 
         *      то аварийно завершит программу с информацией об ошибке.
         */
        const T* operator-> () const {
            ALLOC_LIB_CHECK_NULL_POINTER(_ref._data);
            return reinterpret_cast<const T*>(_ref._data);
        }
    
        /**
         * Оператор индексации.
         * 
         * @remark
         *      Если не объявлен макрос NDEBUG:
         *      
         *      Функция сначала проверит внутренний указатель на nullptr.
         *      И если указатель равен nullptr, 
         *      то аварийно завершит программу с информацией об ошибке.
         * 
         *      Функция проверит диапазон индекса.
         *      И если индекс больше или равна длине аллоцированного блока
         *      то аварийно завершит программу с информацией об ошибке.
         * 
         * @return 
         *     Ссылку на объект по-указанному индексу.
         */
        T& operator[] (std::size_t idx) {
            ALLOC_LIB_CHECK_NULL_POINTER(_ref._data);
            ALLOC_LIB_CHECK_INDEX(idx, _ref.getHeader()->_count);
            return reinterpret_cast<T*>(_ref._data)[idx];
        }
    
        /**
         * Константный оператор индексации.
         * 
         * @remark
         *      Если не объявлен макрос NDEBUG:
         *      
         *      Функция сначала проверит внутренний указатель на nullptr.
         *      И если указатель равен nullptr, 
         *      то аварийно завершит программу с информацией об ошибке.
         * 
         *      Функция проверит диапазон индекса.
         *      И если индекс больше или равна длине аллоцированного блока
         *      то аварийно завершит программу с информацией об ошибке.
         * 
         * @return 
         *     Константную ссылку на объект по-указанному индексу.
         */
        const T& operator[] (std::size_t idx) const {
            ALLOC_LIB_CHECK_NULL_POINTER(_ref._data);
            ALLOC_LIB_CHECK_INDEX(idx, _ref.getHeader()->_count);
            return reinterpret_cast<T*>(_ref._data)[idx];
        }

        T& operator* () {
            ALLOC_LIB_CHECK_NULL_POINTER(_ref._data);
            ALLOC_LIB_CHECK_INDEX(0, _ref.getHeader()->_count);
            return *reinterpret_cast<T*>(_ref._data);
        }

        const T& operator* () const {
            ALLOC_LIB_CHECK_NULL_POINTER(_ref._data);
            ALLOC_LIB_CHECK_INDEX(0, _ref.getHeader()->_count);
            return *reinterpret_cast<const T*>(_ref._data);
        }

        /**
         * @deprecated
         */
        bool isNull() const {
            return _ref._data == nullptr;
        }

        bool is_null() const {
            return isNull();
        }
    };
    
    /**
     * Класс линейного рапределителя, который дефрагментирует кучу.
     * При этом класс является динамически-расширяемым, то есть, куча умеет увеличиваться.
     * 
     * Принцип работы такой:
     * 
     * Есть блок памяти
     * _data        [..........................................]    Блок памяти.
     * _capacity    <------------------------------------------>    Емкость(В байтах).
     * 
     * _offset      [################..........................] 
     *                               ^                              Смещение указателя для выделения.      
     * 
     * При выделении выделяется память не только для фактических данных, но и для заголовка tca::Header
     * Который хранит общий размер выделенной области (Вместе с размером заголовка), 
     * функцию для перемещения (Так как аллокатор знать не знает какие типы он должен аллоцировать),
     * Количество выделенных объектов (Для массивов, например) для использование этой информации в функции перемещения.
     * 
     * А так-же заголовок хранит указатель на владеющий объект-ссылки, чтобы при перемещении блоков памяти обновить указатель на актуальный.
     * Объект-ссылка так же хранит указатель на заголовок, чтобы при вызове деструктора объекта-ссылки пометить блок памяти, как свободный.
     */
    class compact_linear_allocator {
        /**
         * Удалённые конструкторы копирования и присваивания с копированием
         */
        compact_linear_allocator(const compact_linear_allocator&) = delete;
        compact_linear_allocator& operator= (const compact_linear_allocator&) = delete;
        
        //Указатель на базовый аллокатор.
        base_allocator*  _allocator;
        
        //Указатель на блок памяти
        void*           _data;
        
        //Размер блока памяти, на который указывает CompactAllocator::_data 
        std::size_t     _capacity;
        
        //Указатель на блок памяти в который аллоцируются временные объекты при компактизации.
        void*           _tmp;
        
        //Размер блока памяти, на который указывает CompactAllocator::_tmp.
        std::size_t     _tmpCapacity;
    
        //Смещение для выделения новых объектов. 
        std::size_t     _offset;
    
        //Перемещение данных из src в dst
        void moveHeader(Header* dst, Header* src);
        
        //Расширение буфера в два раза.
        void grow();
        
        //Функция для выделения сырых байтов под данные, а так же установки фукнции для перемещения.
        Ref allocate(std::size_t sz, std::size_t count, void (*move_func)(void*, void*, std::size_t));
    
        /**
         * Функция перемещения по-умолчанию.
         * 
         * Так как аллокатор расчитан на то, что может выделять не только примитивные типы, но и сложные объекты,
         * то для сложных объектов необходимо вызывать конструктор перемещения! НЕ ОПЕРАТОР ПЕРЕМЕЩЕНИЯ!
         * 
         * Все объекты перемещаются только с помощью конструктора и конструкции вида:
         * {
         *      for (std::size_t i = 0; i < count; ++i)
         *          new(dst + i) T(std::move( *(src + i) );
         * }
         * 
         * Хотя функция и принимает аргменты, как void*, но так-как функция является шаблонной, в теле функции тип сохраняется.
         * 
         * Почему это нужно?
         * Чтобы сделать tca::Header максимально абстрактным.
         * tca::Header хранит указатель на сырые данные, размер и сколько фактически было выделено объектов и ничего больше.
         * 
         * Но при перемещении блоков нельзя просто вызываться std::memmove, то нужна так же и функция, которая так же не знает тип в параметре, 
         * но знает тип уже внутри своего тела.
         * 
         * Поэтой причине tca::Header добавляет +1 поле, которым является указатель на функцию перемещения. void (*move_func)(void*, void*);
         */
        template<typename T>
        static void mov(void* dst, void* src, std::size_t count);
    
    public:
        /**
         * Создаёт линейный аллокатор используя объект BaseAllocator и начальную емкость.
         * 
         * @param allocator
         *          Указатель на аллокатор из которого будет аллоцироваться память для текущего аллокатора.
         * 
         * @param capacity
         *          Начальная емкость аллокатора.
         */
        compact_linear_allocator(base_allocator* allocator, std::size_t capacity);
        
        /**
         * Перемещает данные из ранее выделенного аллокатора в этот аллокатор.
         */
        compact_linear_allocator(compact_linear_allocator&&);
        
        /**
         * Перемещает данные из ранее выделенного аллокатора в этот аллокатор.
         * При этом вызывая функцию compact_linear_allocator::free()
         */
        compact_linear_allocator& operator= (compact_linear_allocator&&);
    
        /**
         * Очищает выделенную аллокатором память.
         * 
         * @remark
         *      Все нетривиальные типы данных, которые были аллоцированы с помощью этого аллокатора или аллокаторов наследников,
         *      обязаны быть освобождены до вызова этого декстуктора, иначе поведение не определено!
         * 
         */
        ~compact_linear_allocator();
    
        /**
         * Аллоцирует выровненный блок сырой* памяти для объекта T количеством n
         * Сырой* - значит, что у памяти не вызывается конструктор типа.
         * 
         * @param n - количество объектов типа T
         * 
         * @return ссылка на блок памяти.
         */
        template<typename T>
        reference<T> allocate(std::size_t n);
        
        /**
         * Аллоцирует выровненный блок памяти для объекта T количеством n
         * И вызывает конструктор по-умолчанию.
         * 
         * @param n - количество объектов типа T
         * 
         * @return ссылка на блок памяти.
         */
        template<typename T>
        reference<T> allocate_constructed(std::size_t count);
    
        /**
         * Устанавливает новую ёмкость для внутреннего буфера аллокатора. (В байтах)
         * Новый размер обязан быть больше, чем текущая ёмкость.
         * 
         * @param newCapacity - новая ёмкость аллокатора.
         */
        void grow(std::size_t newCapacity);
        
        /**
         * Производит уплотнение выделенных блоков памяти, 
         * чтобы убрать фрагментацию или освободить место для новых аллокаций.
         */
        void compact();
        
        /**
         * Производит полное очищение аллокатора.
         *  @remark
         *      Все нетривиальные типы данных, которые были аллоцированы с помощью этого аллокатора или аллокаторов наследников,
         *      обязаны быть освобождены до вызова этой функции, иначе поведение не определено!
         */
        void free();
    
        /**
         * Выводит на консоль отладочную информацию аллокатора.
         * Функция использует std::printf
         */
        void print() const;
    };  

    template<typename T>
    /*static*/ void compact_linear_allocator::mov(void* dst, void* src, std::size_t count) {
        if (std::is_trivially_copyable<T>::value) {
            memcpy(dst, src, sizeof(T) * count);
        } else {
            T* d = reinterpret_cast<T*>(dst);
            T* s = reinterpret_cast<T*>(src);
            for (std::size_t i = 0; i < count; ++i) {
                new(d + i) T(std::move(*(s + i)));
                s[i].~T();
            }
        }
    }

    template<typename T>
    reference<T> compact_linear_allocator::allocate(std::size_t count) {
        if (count == 0)
            return reference<T>();

        Ref ref = allocate(count * sizeof(T), count, mov<T>);
        
        //Проверка указателя на выравнивание.
        ALLOC_LIB_CHECK_ALIGN(ref._data, T);
   
        return reference<T>(std::move(ref));
    }

    template<typename T>
    reference<T> compact_linear_allocator::allocate_constructed(std::size_t count) {
        if (count == 0)
            return reference<T>();
        
        Ref rawRef = allocate(count * sizeof(T), count, mov<T>);
        
        //Проверка указателя на выравнивание.
        ALLOC_LIB_CHECK_ALIGN(rawRef._data, T);
        
        reference<T> ref(std::move(rawRef));
        
        T* data = (T*) ref;
        for (std::size_t i = 0; i < count; ++i)
            new (data + i) T();
        
        return reference<T>(std::move(ref));
    }

}


#endif//_ALLOCATORS_LINEAR_COMPACT_ALLOCATOR_ALLOCATOR_H