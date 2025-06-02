#ifndef _ALLOCATORS_BASE_ALLOCATOR_H
#define _ALLOCATORS_BASE_ALLOCATOR_H

#include <cctype>

namespace tca {


    
class base_allocator;
namespace internal 
{
    extern thread_local base_allocator* scoped_allocator;
}

    /**
     * Возвращает аллокатор по-умолчанию.
     */
    base_allocator* get_default_allocator();

    /**
     * Возвращает значение scoped_allocator, или get_default_allocator(), если scoped_allocator == nullptr
     */
    base_allocator* get_scoped_or_default();

/**
 * Базовый тип полиморфного распределителя.
 */
class base_allocator {
    /**
     * !!!Операции копирования запрещены!!!
     */
    base_allocator(const base_allocator& base);
    base_allocator& operator= (const base_allocator& base);
protected:
    /**
     * Указатель на родительский объект распределителя.
     */
    base_allocator* parent;
public:    
    /**
     * Создаёт распределитель памяти, инициализируя значениями по-умолчанию.
     */
    base_allocator();
    
    /**
     * Создаёт распределитель памяти с использованием родительского распределителя.
     * 
     * @param parent
     *          Указатель на родительский распределитель.
     */
    base_allocator(base_allocator* parent);
    
    /**
     * Перемещает данные распределителя из передаваемого объекта в этот объект.
     * 
     * @param base
     *          R-value ссылка рапределителя для перемещения данных из base в этот объект.
     */
    base_allocator(base_allocator&& base);
    
    /**
     * Перемещает данные распределителя из передаваемого объекта в этот объект.
     * @param base
     *          R-value ссылка рапределителя для перемещения данных из base в этот объект.
     */
    base_allocator& operator= (base_allocator&& base);
    
    /**
     * В этой версии ничего не делает.
     */
    virtual ~base_allocator() = 0;
    
    /**
     * Выделяет блок памяти размером sz и возвращает на него указатель void*
     * 
     * @remark
     *      Данная функция не беспокоится о выравнивании данных.
     *      Будет ли блок памяти выровнен или нет зависит от реализации класса наследника.
     *      Чтобы аллоцировать блок с выравниванием используйте tca::BaseAllocator::allocateAlign(std::size_t, sd::size_t);
     * 
     * @param sz
     *      Размер блока памяти.
     * 
     * @return 
     *      Указатель на начало блока.
     */
    virtual void* allocate(std::size_t sz) = 0;
    
    /**
     * Выделяет блок памяти размером sz и выравниванием align, 
     * и возвращает на него указатель void*
     * 
     * Данная функция обязана возвращать адрес выровненный не меньше, чем на align
     * 
     * @param sz
     *      Размер блока памяти.
     * 
     * @param align
     *      Выравнивание для выделяемого блока памяти.
     * 
     * @return 
     *      Указатель на начало блока.
     * 
     */
    virtual void* allocate_align(std::size_t sz, std::size_t align) = 0;
    
    /**
     * Особождает выделенную память.
     * 
     * @param ptr
     *      Указатель на выделенный ранее блок памяти.
     * 
     * @param sz
     *      Размер выделенного ранее блока памяти.
     */
    virtual void deallocate(void* ptr, std::size_t sz) = 0;
};

class scope_allocator {
    base_allocator* m_prev;
    scope_allocator(const scope_allocator&) = delete;
    scope_allocator& operator= (const scope_allocator&) = delete;
    scope_allocator(scope_allocator&&) = delete;
    scope_allocator& operator= (scope_allocator&&) = delete;
public:
    scope_allocator(base_allocator* allocator);
    ~scope_allocator();
    base_allocator* get_prev() const;
};

}
#endif//_ALLOCATORS_BASE_ALLOCATOR_H