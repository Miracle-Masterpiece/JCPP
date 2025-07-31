#ifndef _ALLOCATORS_STACK_ALLOCATOR_H
#define _ALLOCATORS_STACK_ALLOCATOR_H

#include <allocators/base_allocator.hpp>
#include <cinttypes>
#include <cassert>
#include <cstdio>

namespace tca {

struct stack_frame_t {
    /**
     * 
     */
    stack_frame_t*      m_prev;
    
    /**
     * 
     */
    std::size_t         m_allocated;
};

class stack_allocator;
class stack_frame {
    /**
     * 
     */
    stack_allocator* stack;
    
    /**
     * 
     */
    stack_frame(const stack_frame&);
    
    /**
     * 
     */
    stack_frame& operator= (const stack_frame&);
public:
    
    /**
     * 
     */
    stack_frame(stack_allocator* stack);
    
    /**
     * 
     */
    stack_frame(stack_frame&&);
    
    /**
     * 
     */
    stack_frame& operator=(stack_frame&&);
    
    /**
     * 
     */
    ~stack_frame();
};

class stack_allocator : public base_allocator {
    
    /**
     * Указатель на начала памяти стека.
     */
    char*           m_data;
    
    /**
     * Размер общей памяти стека.
     */
    size_t          m_data_length;
    
    /**
     * Указатель на последний кадр стека.
     */
    stack_frame_t*  m_last_frame;

    /**
     * 
     */
    std::size_t     m_stack_size;
    
    /**
     * Общее количество выделенных байт.
     */
    std::size_t     m_total_allocated;
public:
    
    /**
     * @param size
     *      Общий размер стека.
     * 
     * @param allocator
     *      Рапределитель памяти для этого стека.
     */
    stack_allocator(std::size_t size, base_allocator* allocator = get_default_allocator());
    
    /**
     * 
     */
    ~stack_allocator();

    /**
     * 
     */
    inline std::size_t calcAlign(void* ptr, std::size_t align){
        const intptr_t p            = (intptr_t) ptr;
        const std::size_t mod       = p & (align - 1);
        return mod == 0 ? 0 : align - mod;
    }

    /**
     * 
     */
    inline std::size_t calc_align_size(std::size_t size, std::size_t align){
        const std::size_t p  = size;
        const std::size_t mod  = p & (align - 1);
        return mod == 0 ? 0 : align - mod;
    }
    
    /**
     * 
     */
    inline std::size_t alignof_size_frame() {
        return sizeof(stack_frame_t) + calc_align_size(sizeof(stack_frame_t), alignof(stack_frame_t));
    }

    /**
     * Кладёт новый кадр стека.
     * 
     * @return
     *      RAII объект, который автоматически во время разрушения снимает кадр стека.
     */
    stack_frame push_frame();
    
    /**
     * Снимает кадр со стека.
     * 
     * @return
     *      true - если указатель на кадр не null, иначе false.
     */
    bool pop_frame();
    
    /**
     * Выделяет память в size байт с выравниванием align.
     * 
     * @param size
     *      Размер запрашиваемой памяти.
     * 
     * @param align
     *      Размер выравнивания памяти.
     * 
     * @return
     *      Указатель на память или nullptr в случае неудачи выделения.
     */
    void* alloc_bytes(std::size_t size, std::size_t align);
    
    /**
     * Выделяет память в size байт с выравниванием alignof(char)
     * 
     * @param sz
     *      Размер запрашиваемой памяти.
     * 
     * @return
     *      Указатель на память или nullptr в случае неудачи выделения.
     */
    void* allocate(std::size_t sz) override;
    
    /**
     * Выделяет память в size байт с выравниванием align.
     * 
     * @param size
     *      Размер запрашиваемой памяти.
     * 
     * @param align
     *      Размер выравнивания памяти.
     * 
     * @return
     *      Указатель на память или nullptr в случае неудачи выделения.
     */
    void* allocate_align(std::size_t sz, std::size_t align) override;
    
    /**
     * 
     */
    void deallocate(void* p, std::size_t sz) override;
    
    /**
     * Распечатывает информацию о конкретном кадре стека.
     * 
     * @param frame
     *      Указатель на кадр стека для которого будет распечатана информация.
     */
    void frame_print(stack_frame_t* frame) const;
    
    /**
     * Распечатывает информацию о стеке.
     */
    void print() const;
};
    
}

#endif//_ALLOCATORS_STACK_ALLOCATOR_H