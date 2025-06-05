#ifndef JSTD_CPP_LANG_UTILS_SHARED_PTR_H
#define JSTD_CPP_LANG_UTILS_SHARED_PTR_H

#include <allocators/base_allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/traits.hpp>
#include <cstdint>
#include <utility>
#include <cstdio>
#include <new>
#include <cassert>

namespace jstd 
{
namespace internal 
{
namespace sptr 
{

    struct shared_control_block {
        
        tca::base_allocator*    m_allocator;
        void*                   m_object;
        uint32_t                m_strong_refs;
        uint32_t                m_weak_refs;
        
        /*Тут располагается объект */
        
        shared_control_block();
        shared_control_block(tca::base_allocator* allocator, void* object);
        ~shared_control_block();

        void inc_strong_ref();
        void dec_strong_ref();

        void inc_weak_ref();
        void dec_weak_ref();

        uint32_t strong_count() const;
        uint32_t weak_count() const;

        static const int32_t TO_STRING_MIN_BUFFER_SIZE = 64;
        int32_t to_string(char buf[], int32_t bufsize) const;
    };
    
    uint32_t calc_ctr_block_total_size(uint32_t data_type_sizeof, uint32_t data_type_alignof, uint32_t* offset_to_object = nullptr);

    /**
     * 
     */
    template<typename T, typename _T>
    shared_control_block* make_control_block(tca::base_allocator* allocator, _T&& obj) {
        uint32_t offset_to_object = 0;
        const uint32_t sz = calc_ctr_block_total_size(sizeof(T), alignof(T), &offset_to_object);
        void* p = allocator->allocate_align(sz, 
                                                alignof(T) > alignof(shared_control_block) ? 
                                                alignof(T) : alignof(shared_control_block));

        assert(((intptr_t) p % alignof(shared_control_block)) == 0);
        
        if (p == nullptr)
            throw_except<out_of_memory_error>("Out of memory");
        
        void* const obj_p = reinterpret_cast<void*>(reinterpret_cast<char*>(p) + offset_to_object);
        assert(((intptr_t) obj_p % alignof(T)) == 0);
        
        try {
            new (obj_p) T(std::forward<_T>(obj));
        } catch (const jstd::throwable& t) {
            allocator->deallocate(p, sz);
            throw t;
        }
        
        shared_control_block* ctrl_block = new(p) shared_control_block(allocator, obj_p);

        return ctrl_block;
    }

} //namespace sptr
} //namespace internal


template<typename T>
class shared_ptr;

template<typename T>
class weak_ptr {
    friend class shared_ptr<T>;
    
    internal::sptr::shared_control_block* m_block;

    weak_ptr(internal::sptr::shared_control_block* ctrl_block);
    void cleanup();

public:
    /**
     * Конструктор по умолчанию. Создаёт пустой weak_ptr.
     */
    weak_ptr();

    /**
     * Конструктор копирования. Увеличивает счётчик слабых ссылок.
     * 
     * @param other 
     *      Другой weak_ptr.
     */
    weak_ptr(const weak_ptr<T>& other);

    /**
     * Конструктор перемещения. Забирает владение у другого weak_ptr.
     * 
     * @param other 
     *      Другой weak_ptr.
     */
    weak_ptr(weak_ptr<T>&& other);

    /**
     * Деструктор. 
     * Уменьшает счётчик слабых ссылок.
     */
    ~weak_ptr();

    /**
     * Оператор присваивания (копирование).
     * 
     * @param other 
     *      Другой weak_ptr.
     * 
     * @return 
     *      *this.
     */
    weak_ptr<T>& operator=(const weak_ptr<T>& other);

    /**
     * Оператор присваивания (перемещение).
     * 
     * @param other 
     *      Другой weak_ptr.
     * 
     * @return 
     *      *this.
     */
    weak_ptr<T>& operator=(weak_ptr<T>&& other);

    /**
     * Возвращает количество активных shared_ptr, владеющих объектом.
     * 
     * @return 
     *      Счётчик сильных ссылок.
     */
    uint32_t use_count() const;

    /**
     * Создаёт shared_ptr из weak_ptr, если объект всё ещё существует.
     * 
     * @return 
     *      Новый shared_ptr или пустой, если объект уничтожен.
     */
    shared_ptr<T> lock() const;

};

template<typename T>
class shared_ptr {
    template<typename A>
    friend class shared_ptr;
    friend class weak_ptr<T>;
    
    internal::sptr::shared_control_block* m_block;
    void cleanup();
    void check_access() const;

    shared_ptr(internal::sptr::shared_control_block* ctrl_block);

public:
    /**
     * Конструктор по умолчанию. Создаёт пустой shared_ptr.
     */
    shared_ptr();

    /**
     * Конструктор с размещением объекта.
     * 
     * @tparam _T 
     *      Тип передаваемого объекта.
     * 
     * @param allocator 
     *      Аллокатор для размещения объекта и блока управления.
     * 
     * @param obj 
     *      Объект, который будет скопирован или перемещён.
     */
    template<typename _T>
    shared_ptr(tca::base_allocator* allocator, _T&& obj);

    /**
     * Конструктор копирования. 
     * Увеличивает счётчик сильных ссылок.
     */
    shared_ptr(const shared_ptr<T>& other);

    /**
     * Конструктор перемещения. 
     * Забирает владение у другого shared_ptr.
     * 
     * @param 
     *      other Другой shared_ptr.
     */
    shared_ptr(shared_ptr<T>&& other);

    /**
     * Оператор присваивания (копирование). Увеличивает счётчик сильных ссылок.
     * 
     * @param other 
     *      Другой shared_ptr.
     * 
     * @return 
     *      *this.
     */
    shared_ptr<T>& operator=(const shared_ptr<T>& other);

    /**
     * Оператор присваивания (перемещение). Забирает владение у другого shared_ptr.
     * 
     * @param other 
     *      Другой shared_ptr.
     * 
     * @return 
     *      *this.
     */
    shared_ptr<T>& operator=(shared_ptr<T>&& other);

    /**
     * Деструктор. 
     * Уменьшает счётчик сильных ссылок. При обнулении уничтожает объект.
     */
    ~shared_ptr();

    /**
     * Разыменование указателя.
     * 
     * @return 
     *      Ссылка на объект.
     * 
     * @throws null_pointer_exception 
     *      Eсли объект не инициализирован.
     */
    T& operator*() const;

    /**
     * Доступ к членам объекта.
     * 
     * @return 
     *      Указатель на объект.
     * 
     * @throws null_pointer_exception 
     *      Eсли объект не инициализирован.
     */
    T* operator->() const;

    /**
     * Возвращает сырой указатель на объект.
     * 
     * @return 
     *      Указатель или nullptr.
     */
    T* get() const;

    /**
     * Проверка на инициализацию.
     * 
     * @return 
     *      true, если shared_ptr указывает на объект.
     */
    operator bool() const;

    /**
     * Возвращает weak_ptr для этого shared_ptr
     * 
     * @return 
     *      weak_ptr, указывающий на этот объект.
     */
    operator weak_ptr<T>() const;

    /**
     * Возвращает количество shared_ptr, владеющих объектом.
     * 
     * @return 
     *      Счётчик сильных ссылок.
     */
    uint32_t use_count() const;

    /**
     * Создаёт weak_ptr, наблюдающий за тем же объектом.
     * 
     * @return 
     *      Новый weak_ptr.
     */
    weak_ptr<T> get_weak() const;

    /**
     * Преобразует shared_ptr<T> в shared_ptr<E> с использованием static_cast.
     * 
     * @tparam E 
     *      Целевой тип.
     * 
     * @return 
     *      Новый shared_ptr с преобразованным типом.
     */
    template<typename E>
    shared_ptr<E> static_pointer_cast() const;

    /**
     * Преобразует shared_ptr<T> в shared_ptr<E> с использованием reinterpret_cast.
     * 
     * @tparam E 
     *      Целевой тип.
     * 
     * @return 
     *      Новый shared_ptr с преобразованным типом.
     */
    template<typename E>
    shared_ptr<E> reinterpret_pointer_cast() const;

    /**
     * Преобразует shared_ptr<T> в shared_ptr<E> с использованием dynamic_cast.
     * 
     * @tparam E 
     *      Целевой тип.
     * 
     * @return 
     *      Новый shared_ptr, или пустой, если преобразование не удалось.
     */
    template<typename E>
    shared_ptr<E> dynamic_pointer_cast() const;

    /**
     * Преобразует shared_ptr<T> в shared_ptr<E> с использованием const_cast.
     * 
     * @tparam E 
     *      Целевой тип.
     * 
     * @return 
     *      Новый shared_ptr с преобразованным типом.
     */
    template<typename E>
    shared_ptr<E> const_pointer_cast() const;

    const static int32_t TO_STRING_MIN_BUFFER_SIZE = 48;
    /**
     * Возвращает отладочную строку о состоянии shared_ptr.
     * 
     * @param buf 
     *      Буфер для записи.
     * 
     * @param bufsize 
     *      Размер буфера.
     * 
     * @return 
     *      Количество записанных символов.
     */
    int32_t to_string(char buf[], int32_t bufsize) const;
};


    template<typename T>
    shared_ptr<T>::shared_ptr() : m_block(nullptr) {

    }
    
    template<typename T>
    shared_ptr<T>::shared_ptr(internal::sptr::shared_control_block* ctrl_block) : m_block(ctrl_block) {
        if (m_block != nullptr)
            m_block->inc_strong_ref();
    }

    template<typename T>
    template<typename _T>
    shared_ptr<T>::shared_ptr(tca::base_allocator* allocator, _T&& obj) {
        m_block = internal::sptr::make_control_block<T>(allocator, std::forward<_T>(obj));
        assert(m_block != nullptr);
        m_block->inc_strong_ref();
    }

    template<typename T>
    shared_ptr<T>::shared_ptr(const shared_ptr<T>& ptr) : m_block(ptr.m_block) {
        if (m_block != nullptr)
            m_block->inc_strong_ref();
    }

    template<typename T>
    shared_ptr<T>::shared_ptr(shared_ptr<T>&& ptr) : m_block(ptr.m_block) {
        if (m_block != nullptr)
            m_block->inc_strong_ref();
    }
    
    template<typename T>
    shared_ptr<T>& shared_ptr<T>::operator= (const shared_ptr<T>& ptr) {
        if (&ptr != this) {
            cleanup();
            m_block = ptr.m_block;
            if (m_block != nullptr)
                m_block->inc_strong_ref();
        }
        return *this;
    }

    template<typename T>
    shared_ptr<T>& shared_ptr<T>::operator= (shared_ptr<T>&& ptr) {
        if (&ptr != this) {
            cleanup();
            m_block     = ptr.m_block;
            ptr.m_block = nullptr;
        }
        return *this;
    }

    template<typename T>
    void shared_ptr<T>::cleanup() {
        if (m_block != nullptr) {
            if (m_block->strong_count() == 1) {
                T* obj = reinterpret_cast<T*>(m_block->m_object);
                assert(obj != nullptr);
                obj->~T();

            if (m_block->weak_count() == 0) {
                tca::base_allocator* allocator = m_block->m_allocator;
                assert(allocator != nullptr);
                allocator->deallocate(m_block, internal::sptr::calc_ctr_block_total_size(sizeof(T), alignof(T)));
            }

            } else if (m_block->strong_count() > 0) {
                m_block->dec_strong_ref();
            }
        }
    }
    
    template<typename T>
    shared_ptr<T>::~shared_ptr() {
        cleanup();
    }

    template<typename T>
    void shared_ptr<T>::check_access() const {
#ifndef NDEBUG
        if (m_block == nullptr)
            throw_except<null_pointer_exception>("pointer must be != null");
#endif//NDEBUG
    }

    template<typename T>
    T& shared_ptr<T>::operator*() const {
        check_access();
        return *((T*) m_block->m_object);
    }
    
    template<typename T>
    T* shared_ptr<T>::operator->() const {
        check_access();
        return (T*) m_block->m_object;
    }

    template<typename T>
    uint32_t shared_ptr<T>::use_count() const {
        if (m_block != nullptr)
            return m_block->strong_count();
        return 0;
    }

    template<typename T>
    shared_ptr<T>::operator bool() const {
        return get() != nullptr;
    }

    template<typename T>
    shared_ptr<T>::operator weak_ptr<T>() const {
        return get_weak();
    }

    template<typename T>
    T* shared_ptr<T>::get() const {
        if (m_block == nullptr || m_block->strong_count() == 0)
            return nullptr;
        return (T*) m_block->m_object;
    }

    template<typename T>
    weak_ptr<T> shared_ptr<T>::get_weak() const {
        return weak_ptr<T>(m_block);
    }

    template<typename T>
    template<typename E>
    shared_ptr<E> shared_ptr<T>::static_pointer_cast() const {
        if (m_block == nullptr || m_block->m_object == nullptr)
            return shared_ptr<E>();
        return shared_ptr<E>(m_block, static_cast<E*>(reinterpret_cast<T*>(m_block->m_object)));
    }

    template<typename T>
    template<typename E>
    shared_ptr<E> shared_ptr<T>::reinterpret_pointer_cast() const {
        if (m_block == nullptr || m_block->m_object == nullptr)
            return shared_ptr<E>();
        return shared_ptr<E>(m_block, nullptr);
    }

    template<typename T>
    template<typename E>
    shared_ptr<E> shared_ptr<T>::dynamic_pointer_cast() const {
        if (m_block == nullptr || m_block->m_object == nullptr)
            return shared_ptr<E>();
        return shared_ptr<E>(m_block, dynamic_cast<E*>(reinterpret_cast<T*>(m_block->m_object)));
    }

    template<typename T>
    template<typename E>
    shared_ptr<E> shared_ptr<T>::const_pointer_cast() const {
        if (m_block == nullptr || m_block->m_object == nullptr)
            return shared_ptr<E>();
        return shared_ptr<E>(m_block, const_cast<E*>(reinterpret_cast<T*>(m_block->m_object)));
    }

    template<typename T>
    int32_t shared_ptr<T>::to_string(char buf[], int32_t bufsize) const {
        if (m_block != nullptr) {
            return m_block->to_string(buf, bufsize);
        } else {
            return snprintf(buf, bufsize, "null");
        }
    }

    template<typename T, typename _T>
    shared_ptr<T> make_shared(_T&& obj, tca::base_allocator* allocator = tca::get_scoped_or_default()) {
        return shared_ptr<T>(allocator, std::forward<_T>(obj));
    }

    /**
     * #####################################################################################
     * 
     *                                  W E A K _ P T R
     * 
     * #####################################################################################
     */


    template<typename T>
    weak_ptr<T>::weak_ptr(internal::sptr::shared_control_block* ctrl_block) : m_block(ctrl_block) {
        if (ctrl_block != nullptr)
            ctrl_block->inc_weak_ref();
    }
    
    template<typename T>
    void weak_ptr<T>::cleanup() {
        if (m_block != nullptr) {
            m_block->dec_weak_ref();
            if (m_block->strong_count() == 0 && m_block->weak_count() == 0) {
                tca::base_allocator* allocator = m_block->m_allocator;
                m_block->~shared_control_block();
                assert(allocator != nullptr);
                allocator->deallocate(m_block, internal::sptr::calc_ctr_block_total_size(sizeof(T), alignof(T)));
            }
        }
    }

    template<typename T>
    weak_ptr<T>::weak_ptr() : m_block(nullptr) {

    }
    
    template<typename T>
    weak_ptr<T>::~weak_ptr() {
        cleanup();
    }

    template<typename T>
    weak_ptr<T>::weak_ptr(const weak_ptr<T>& ptr) : m_block(ptr.m_block) {
        if (m_block != nullptr)
            m_block->inc_weak_ref();
    }
    
    template<typename T>
    weak_ptr<T>::weak_ptr(weak_ptr<T>&& ptr) : m_block(ptr.m_block) {
        ptr.m_block = nullptr;
    }
    
    template<typename T>
    weak_ptr<T>& weak_ptr<T>::operator=(const weak_ptr<T>& ptr) {
        if (&ptr != this) {
            cleanup();
            m_block = ptr.m_block;
            if (m_block != nullptr)
                m_block->inc_weak_ref();
        }
        return *this;
    }
    
    template<typename T>
    weak_ptr<T>& weak_ptr<T>::operator=(weak_ptr<T>&& ptr) {
        if (&ptr != this) {
            cleanup();
            m_block     = ptr.m_block;
            ptr.m_block = nullptr;
        }
        return *this;
    }

    template<typename T>
    uint32_t weak_ptr<T>::use_count() const {
        if (m_block == nullptr)
            return 0;
        return m_block->strong_count();
    }
    
    template<typename T>
    shared_ptr<T> weak_ptr<T>::lock() const {
        if (m_block == nullptr || m_block->strong_count() == 0)
            return shared_ptr<T>();
        return shared_ptr<T>(m_block);
    }

} //namespace jstd

#endif//JSTD_CPP_LANG_UTILS_SHARED_PTR_H