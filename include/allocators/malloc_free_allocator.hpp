#ifndef _ALLOCATORS_MALLOC_FREE_ALLOCATOR_H
#define _ALLOCATORS_MALLOC_FREE_ALLOCATOR_H

#include <allocators/base_allocator.hpp>

namespace tca {

/**  
 * @class malloc_free_allocator  
 * @brief Аллокатор, использующий стандартные функции выделения и освобождения памяти malloc/free.  
 *  
 * Данный аллокатор выделяет память через malloc() и aligned_alloc() (с alignof(std::max_align_t))  
 * и освобождает её с помощью free(). Он не отслеживает выделенные блоки и предназначен для простого  
 * управления памятью в стиле стандартного malloc/free.  
 */  
class malloc_free_allocator : public base_allocator {  
public:  
    
    /**  
     * Перераспределяет память с использованием realloc.
     * 
     * @param p
     *      Указатель на ранее выделенный блок.
     * 
     * @param new_size
     *      Новый размер блока памяти в байтах.  
     * 
     * @return 
     *          Указатель на выделенную память или nullptr, если выделение не удалось.  
     */  
    void* reallocate(void* p, std::size_t new_size);

    /**  
     * Выделяет sz байтов с использованием malloc(). Выравнивание определяется системой  
     * и гарантированно совместимо со всеми стандартными типами.  
     *  
     * @param sz 
     *          Размер блока памяти в байтах.  
     * 
     * @return 
     *          Указатель на выделенную память или nullptr, если выделение не удалось.  
     */  
    void* allocate(std::size_t sz) override;  

    /**  
     * Выделяет sz байтов памяти с выравниванием, равным alignof(std::max_align_t).  
     * Если запрашиваемый размер sz не кратен alignof(std::max_align_t), он будет скорректирован.  
     *  
     * @param sz 
     *          Размер блока памяти в байтах.  
     * 
     * @param align 
     *          Запрашиваемое выравнивание (игнорируется, используется alignof(std::max_align_t)).  
     * 
     * @return 
     *          Указатель на выделенную память или nullptr, если выделение не удалось.  
     */  
    void* allocate_align(std::size_t sz, std::size_t align) override;  

    /**  
     * Освобождает блок памяти, выделенный через allocate() или allocateAlign(),  
     * с использованием free().  
     *  
     * @param p 
     *          Указатель на освобождаемый блок памяти.  
     * 
     * @param sz 
     *          Размер блока памяти (может игнорироваться в реализации).  
     */  
    void deallocate(void* p, std::size_t sz) override;  
};

}

#endif//_ALLOCATORS_MALLOC_FREE_ALLOCATOR_H