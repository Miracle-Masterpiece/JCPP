#ifndef TCA_ALLOCATORS_BASE_ALLOCATOR_H
#define TCA_ALLOCATORS_BASE_ALLOCATOR_H

#include <allocators/shared_allocator.hpp>

namespace tca
{

/**
 * Реализация аллокатора на базе арены с использованием списка свободных блоков (free list).
 * Аллокатор резервирует непрерывный блок памяти фиксированного размера и управляет им вручную,
 * поддерживая список свободных блоков, которые могут быть повторно использованы.
 * Не поддерживает автоматическое расширение. При нехватке памяти возвращает nullptr.
 * Поддерживает выравнивание и может быть сброшен или компактирован вручную.
 */
class arena_free_list_allocator : public tca::base_allocator {
    arena_free_list_allocator(const arena_free_list_allocator&) = delete;
    arena_free_list_allocator& operator= (const arena_free_list_allocator&) = delete;
    internal::chunk m_arena;
public:

    /**
     * Создаёт новый аллокатор с ареной заданного размера.
     *
     * @param arena_size 
     *      Размер арены в байтах. Должен быть больше 0.
     * 
     * @param allocator  
     *      Базовый аллокатор, используемый для выделения самой арены. По умолчанию используется текущий scoped-аллокатор.
     */
    arena_free_list_allocator(std::size_t arena_size, base_allocator* allocator = get_scoped_or_default());

    /**
     * Перемещающий конструктор. Передаёт владение ареной и состоянием.
     */
    arena_free_list_allocator(arena_free_list_allocator&&);

    /**
     * Перемещающее присваивание. Предыдущая арена будет освобождена.
     */
    arena_free_list_allocator& operator= (arena_free_list_allocator&&);

    /**
     * Освобождает арену и все связанные с ней ресурсы.
     */
    ~arena_free_list_allocator();

    /**
     * Выделяет память заданного размера из арены.
     *
     * @param sz 
     *      Размер в байтах.
     * 
     * @return 
     *      Указатель на выделенную память или nullptr, если недостаточно места.
     */
    void* allocate(std::size_t sz) override;

    /**
     * Выделяет выровненную память из арены.
     *
     * @param sz    
     *      Размер в байтах.
     * 
     * @param align 
     *      Требуемое выравнивание. Должно быть степенью двойки.
     * 
     * @return 
     *      Указатель на выделенную память или nullptr, если недостаточно места.
     */
    void* allocate_align(std::size_t sz, std::size_t align) override;

    /**
     * Освобождает ранее выделенный блок памяти.
     * Блок должен быть ранее выделен этим же аллокатором.
     *
     * @param p  
     *      Указатель на блок.
     * 
     * @param sz 
     *      Размер блока в байтах.
     */
    void deallocate(void* p, std::size_t sz) override;

    /**
     * Сбрасывает арену, делая всю память доступной для повторного использования.
     * Вся ранее выделенная память считается недействительной.
     */
    void reset();

    /**
     * Компактирует свободные блоки, объединяя смежные.
     * Уменьшает фрагментацию и улучшает возможность будущих аллокаций.
     * Не перемещает занятые блоки.
     */
    void compact();

    /**
     * Печатает отладочную информацию о текущем состоянии арены.
     * Включает количество блоков, размеры и информацию о том, какие из них свободны.
     */
    void print() const;
};


}

#endif//TCA_ALLOCATORS_BASE_ALLOCATOR_H