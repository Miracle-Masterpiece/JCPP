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
    base_allocator(const base_allocator& base)              = delete;
    base_allocator& operator= (const base_allocator& base)  = delete;
protected:
    /**
     * Указатель на родительский объект распределителя.
     */
    base_allocator* m_parent;
public:    
    /**
     * Создаёт распределитель памяти с использованием родительского распределителя.
     * Если не указан явно, будет использоваться распределитель по-умолчанию.
     * 
     * @param parent
     *          Указатель на родительский распределитель. (Опционально)
     *          
     */
    base_allocator(base_allocator* parent = get_default_allocator());
    
    /**
     * Перемещает данные распределителя из передаваемого объекта в этот объект.
     * 
     * @param base
     *          R-value ссылка рапределителя для перемещения данных из base в этот объект.
     */
    base_allocator(base_allocator&& base);
    
    /**
     * Перемещает данные распределителя из передаваемого объекта в этот объект.
     * 
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
     *      Чтобы аллоцировать блок с выравниванием используйте tca::base_allocator::allocate_align(std::size_t, sd::size_t);
     * 
     * @param sz
     *      Размер блока памяти.
     * 
     * @return 
     *      Указатель на начало блока.
     * 
     * @version 1.1
     *      По-умолчанию делегирует аллокацию в объект m_parent
     */
    virtual void* allocate(std::size_t sz);
    
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
     * @version 1.1
     *      По-умолчанию делегирует аллокацию в объект m_parent
     */
    virtual void* allocate_align(std::size_t sz, std::size_t align);
    
    /**
     * Особождает выделенную память.
     * 
     * @param ptr
     *      Указатель на выделенный ранее блок памяти.
     * 
     * @param sz
     *      Размер выделенного ранее блока памяти.
     * 
     * @version 1.1
     *      По-умолчанию делегирует аллокацию в объект m_parent
     */
    virtual void deallocate(void* ptr, std::size_t sz);
};

/**
 * Класс для установки некого аллокатора, как общего для текущей области видимости.
 * 
 * @code {
 *      #include <allocators/base_allocator.hpp>
 *      #include <allocators/inline_linear_allocator.hpp>
 *      int main() {
 *          int* ip = (int*) tca::get_scoped_or_default()->allocate(sizeof(int)); // вызывает распределитель установленный по-умолчанию
 *          tca::get_scoped_or_default()->deallocate(ip, sizeof(int));
 * 
 *          tca::inline_linear_allocator<1024> stack_linear_allocator;
 *          tca::scope_allocator scope = &stack_linear_allocator;
 *          ip = (int*) tca::get_scoped_or_default()->allocate(sizeof(int)); // вызывает allocate у stack_linear_allocator
 *      }
 * }
 * 
 * Установка для текущей области видимости является thread-local и разная у каждого потока.
 */
class scope_allocator {
    /**
     * Указатель на предыдущий распределитель.
     */
    base_allocator* m_prev;
    scope_allocator(const scope_allocator&)             = delete;
    scope_allocator& operator= (const scope_allocator&) = delete;
    scope_allocator(scope_allocator&&)                  = delete;
    scope_allocator& operator= (scope_allocator&&)      = delete;
public:
    /**
     * @param allocator
     *      Указатель на распределитель, который будет установлен как аллокатор для текущей области видимости.
     */
    scope_allocator(base_allocator* allocator);
    
    ~scope_allocator();
    
    /**
     * Возвращает указатель на распределитель, установленный в предыдущей области видимости.
     */
    base_allocator* get_prev() const;
};

}
#endif//_ALLOCATORS_BASE_ALLOCATOR_H