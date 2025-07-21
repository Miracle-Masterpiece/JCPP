#ifndef JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_T_H
#define JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_T_H

#include <allocators/Helpers.hpp>
#include <allocators/base_allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/traits.hpp>
#include <cstdint>
#include <utility>
#include <cstdio>
#include <new>
#include <cassert>
#include <cpp/lang/utils/arrays.h>

namespace jstd 
{
namespace internal 
{
namespace sptr 
{

    /**
     * @internal
     */
    struct shared_control_block {
        
        tca::base_allocator*    m_allocator;
        void*                   m_object;
        uint32_t                m_blocksize;
        uint32_t                m_strong_refs;
        uint32_t                m_weak_refs;

        shared_control_block();
        shared_control_block(tca::base_allocator* allocator, void* object, uint32_t blocksize);
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
     * @internal
     * 
     * Выделяет память под контролирующий блок и объект в одной области памяти.
     * 
     * @note
     *      Данная функция не вызывает конструкторы объектов!
     * 
     * @param allocator
     *      Распределитель памяти под котролирующий блок и объект.
     * 
     * @param object_size
     *      sizeof объекта.
     * 
     * @param object_align
     *      alignof объект.
     * 
     * @param n_objects
     *      Количество объектов
     * 
     * @return
     *      Указатель на контролирующий блок или nullptr, если выделение не удалось.
     */
    shared_control_block* alloc_memory_to_control_block(tca::base_allocator* allocator, uint32_t object_size, uint32_t object_align, uint32_t n_objects = 1);

    /**
     * 
     */
    template<typename T, typename _T>
    shared_control_block* make_control_block(tca::base_allocator* allocator, _T&& obj) {
        shared_control_block* ctrl_block = alloc_memory_to_control_block(allocator, sizeof(T), alignof(T));
        if (!ctrl_block)
            return nullptr;
        try {
            assert((std::intptr_t) ctrl_block->m_object % alignof(T) == 0);
            new (ctrl_block->m_object) T(obj);
        } catch (...) {
            using u32 = uint32_t;
            const u32 size = ctrl_block->m_blocksize;
            allocator->deallocate(ctrl_block, size);
            throw;
        }
        return ctrl_block;
    }

    /**
     * 
     */
    template<typename T>
    shared_control_block* make_control_block_array(tca::base_allocator* allocator, uint32_t length) {
        shared_control_block* ctrl_block = alloc_memory_to_control_block(allocator, sizeof(T), alignof(T), length);
        if (!ctrl_block)
            return nullptr;
        try {
            assert((std::intptr_t) ctrl_block->m_object % alignof(T) == 0);
            using non_const_T = typename remove_const<T>::type;
            placement_new<non_const_T>(reinterpret_cast<non_const_T*>(ctrl_block->m_object), length);
        } catch (...) {
            using u32 = uint32_t;
            const u32 size = ctrl_block->m_blocksize;
            allocator->deallocate(ctrl_block, size);
            throw;
        }
        return ctrl_block;
    }

} //namespace sptr
} //namespace internal


template<typename T>
class shared_ptr;

template<typename T>
class weak_ptr {
    friend class shared_ptr<T>;
    
    /**
     * Указатель на управляющий блок.
     */
    internal::sptr::shared_control_block* m_block;

    /**
     * Конструктор для внутреннего API!
     * 
     * @param ctr_block
     *      Указатель на блок памяти, владеть которым будет этот shared_ptr.
     * 
     * Данный конструктор должен увеличивать счётчик weak-reference.
     */
    weak_ptr(internal::sptr::shared_control_block* ctrl_block);
    
    /**
     * Выполняет очищение объекта.
     */
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
    
    /**
     * Указатель на управляющий блок.
     */
    internal::sptr::shared_control_block* m_block;
    
    /**
     * Выполняет очищение объекта.
     */
    void cleanup();
    
    /**
     * Проверяет доступность объекта.
     * 
     * @throws null_pointer_exception
     *      Если указатель на блок равен nullptr.
     * 
     * @throw illegal_state_exception
     *      Если значение strong-reference внутри управляющего блока равно 0.     
     */
    void check_access() const;

public:
    /**
     * Конструктор по умолчанию. Создаёт пустой shared_ptr.
     */
    shared_ptr();

    /**
     * !@internal
     * 
     * Конструктор для внутреннего API!
     * 
     * @param ctr_block
     *      Указатель на блок памяти, владеть которым будет этот shared_ptr.
     * 
     * Данный конструктор увеличивает счётчик strong-reference, если ctrl_block != nullptr
     */
    explicit shared_ptr(internal::sptr::shared_control_block* ctrl_block);

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
     * Приводит shared_ptr к сырому указателю.
     * 
     * @return 
     *      Значение указателя.
     */
    operator T*() const;

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
        if (m_block == nullptr)
            throw_except<out_of_memory_error>("Out of memory");
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
            m_block->dec_strong_ref();
            if (m_block->strong_count() == 0) {
                T* obj = reinterpret_cast<T*>(m_block->m_object);
                assert(obj != nullptr);
                obj->~T();
                if (m_block->weak_count() == 0) {
                    tca::base_allocator* allocator = m_block->m_allocator;
                    if (allocator != nullptr) //если распределитель равен null, значит за выделение контрол блока отвечает shared_ptr
                        allocator->deallocate(m_block, m_block->m_blocksize);
                }
            }
        }
    }
    
    template<typename T>
    shared_ptr<T>::~shared_ptr() {
        cleanup();
    }

    template<typename T>
    void shared_ptr<T>::check_access() const {
        JSTD_DEBUG_CODE(
            if (m_block == nullptr)
                throw_except<null_pointer_exception>("pointer must be != null");
        );
    }

    template<typename T>
    shared_ptr<T>::operator T*() const {
        check_access();
        return (T*) (m_block->m_object);
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
        return shared_ptr<E>(m_block);
    }

    template<typename T>
    template<typename E>
    shared_ptr<E> shared_ptr<T>::reinterpret_pointer_cast() const {
        if (m_block == nullptr || m_block->m_object == nullptr)
            return shared_ptr<E>();
        return shared_ptr<E>(m_block);
    }

    template<typename T>
    template<typename E>
    shared_ptr<E> shared_ptr<T>::dynamic_pointer_cast() const {
        if (m_block == nullptr || m_block->m_object == nullptr)
            return shared_ptr<E>();
        return shared_ptr<E>(m_block);
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
    shared_ptr<T> make_shared(_T&& obj = T(), tca::base_allocator* allocator = tca::get_scoped_or_default()) {
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
                if (allocator != nullptr)
                    allocator->deallocate(m_block, m_block->m_blocksize);
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
        if (!m_block || m_block->strong_count() == 0)
            return shared_ptr<T>();
        return shared_ptr<T>(m_block);
    }
} //namespace jstd

#endif//JSTD_INTERNAL_SMART_PTRS_SHARED_PTR_T_H