#ifndef _TCA_ALLOCATORS_SHARED_ALLOCATOR_H
#define _TCA_ALLOCATORS_SHARED_ALLOCATOR_H

#include <allocators/base_allocator.hpp>
#include <allocators/ArrayList.h>
#include <allocators/Helpers.hpp>
#include <cstdint>
#include <cstddef>

namespace tca {


namespace internal {

typedef unsigned char byte_t;

struct chunk;

/**
 * Структура для хранения информации о выделенном блоке памяти.
 */
struct memblock {
    #ifndef NDEBUG
        static const unsigned long long int DEBUG_NUMBER = 0xCAFEDEADBABE;
        unsigned long long int magic_;
    #endif
    
    std::size_t len_;       // Длина выделенной области.
    memblock*   next_;      // Указатель на следующий свободный блок.
    
    union {
        memblock*   prev_;      // Указатель на предыдущий свободный блок.
        /**
         * Чтобы проверить, выделен ли блок, нужно сверить указатель на указатель chunk::this
         * В случае, если блок не свободен, он указывает на this, иначе на предыдущий элемент(или nullptr).
         */
        chunk*      owner_;     // Или указатель на владельца. (Используется, когда блок выделен).
    };
    
    memblock() : 
    len_(0), 
    next_(nullptr), 
    prev_(nullptr){
        #ifndef NDEBUG
            magic_ = DEBUG_NUMBER;
        #endif
    }
};

struct chunk {
    static const unsigned int ALIGN             = alignof(std::max_align_t);
    static const unsigned int HEADER_SIZE       = calcAlignSize(sizeof(memblock), ALIGN);
    static const unsigned int MIN_ALLOC_SIZE    = alignof(std::max_align_t);

    byte_t*         data_;          // Указатель на блок памяти.
    std::size_t     chunk_size_;    // Размер блока data_
    memblock*       freelist_;      // Список свободных блоков.
    base_allocator* allocator_;     // Внешний аллокатор.
    int64_t         allocs_;         // Количество активных выделенных блоков.

    chunk();
    chunk(byte_t* block, std::size_t sz, base_allocator* allocator);
    chunk(chunk&&);
    chunk& operator= (chunk&&);
    void cleaunup();
    ~chunk();
    void link(memblock*);
    void unlink(memblock*);
    memblock* split(memblock*, std::size_t sz);

    /**
     * Если внутри чанка указатель data_ равен нулю, значит выделение не произошло.
     */
    static chunk make(base_allocator* allocator, std::size_t sz);

    void* allocate_first_fit(std::size_t sz);
    void* allocate_best_fit(std::size_t sz);
    void free(void* p);
    void compact();
    bool is_empty() const;
    
    /**
     * Возвращает указатель на данные, добавляя смещение на размер заголовка к указателю на memblock
     * 
     * @param block
     *      Указатель на заголовок блока памяти к которому будет добавлено смещение.
     * 
     * @return 
     *      Указатель на блок памяти.
     */
    static void* mem_to_data(memblock* block) {
        return reinterpret_cast<void*>(reinterpret_cast<byte_t*>(block) + chunk::HEADER_SIZE);
    }
    
    /**
     * Возвращает заголовок памяти для передаваемого указателя.
     * 
     * @param p
     *      Указатель на блок данных.
     * 
     * @return
     *      Заголовок памяти передаваемого указателя.
     */
    static memblock* data_to_mem(void* p) {
        return reinterpret_cast<memblock*>(reinterpret_cast<byte_t*>(p) - chunk::HEADER_SIZE);
    }
    
    /**
     * Обновляет ссылки на owher_ внутри блоков чанков.
     * Указатель обновляется только у выделенных блоков.
     */
    void update_this_poitner(chunk* old_owner);
    
    void print_log() const;
private:
    chunk(const chunk&) = delete;
    chunk& operator= (const chunk&) = delete;
};

}


/**
 * Класс реализует механизм управления памятью на основе чанков.
 * Позволяет эффективно выделять и освобождать память, используя заданную стратегию,
 * а также контролировать минимальный размер и выравнивание чанков.
 * 
 * Аллокатор работает поверх базового аллокатора tca::base_allocator и управляет
 * списком внутренних чанков.
 * 
 * Объекты этого класса не копируются - только перемещаются.
 */
class shared_allocator : public base_allocator {

    /**
     * Минимальный размер чанка по умолчанию (128 КБ).
     * Используется, если не указано иное.
     */
    static const std::size_t DEFAULT_MIN_CHUNK_SIZE = 0x20000;

    // Указатель на базовый аллокатор, через который происходит физическое выделение памяти.
    tca::base_allocator* allocator_;

    // Список чанков, которыми управляет данный аллокатор.
    array_list<internal::chunk> chunk_list_;

    /**
     * Минимальное выравнивание размеров чанков.
     * Это значение не влияет на выравнивание самих блоков, возвращаемых {@code allocate()}.
     * Используется только для округления размера чанков вверх до ближайшего кратного.
     */
    std::size_t min_chunk_aligned_;

public:

    /**
     * Стратегия выделения блоков памяти.
     * - {@code FIRST_FIT} — выделяется первый подходящий блок.
     * - {@code BEST_FIT} — ищется наиболее подходящий по размеру блок.
     */
    enum alloc_strategy {
        BEST_FIT, FIRST_FIT
    };

    // Текущая стратегия аллокации.
    alloc_strategy strategy_;

    /**
     * Конструктор по умолчанию.
     * Использует стратегию {@code FIRST_FIT} и выравнивание по умолчанию.
     */
    shared_allocator();

    /**
     * Конструктор с указанием базового аллокатора и стратегии выделения.
     *
     * @param allocator 
     *      Базовый аллокатор.
     * 
     * @param strategy 
     *      Стратегия выделения блоков.
     */
    shared_allocator(base_allocator* allocator, alloc_strategy strategy_);

    /**
     * Конструктор с полной настройкой: базовый аллокатор, минимальный размер чанка и стратегия.
     *
     * @param allocator 
     *      Базовый аллокатор.
     * 
     * @param min_chunk_aligned 
     *      Минимальное выравнивание размеров чанков.
     * 
     * @param strategy 
     *      Стратегия выделения блоков.
     */
    shared_allocator(base_allocator* allocator, std::size_t min_chunk_aligned = DEFAULT_MIN_CHUNK_SIZE, alloc_strategy strategy_ = alloc_strategy::FIRST_FIT);

    /**
     * Перемещающий конструктор.
     * Позволяет передать владение ресурсами другому экземпляру.
     */
    shared_allocator(shared_allocator&&);

    /**
     * Перемещающее присваивание.
     */
    shared_allocator& operator= (shared_allocator&&);

    /**
     * Деструктор.
     * Освобождает все управляемые ресурсы.
     */
    ~shared_allocator();

    /**
     * Устанавливает стратегию выделения блоков.
     *
     * @param type 
     *      Стратегия выделения.
     */
    void set_alloc_type(alloc_strategy type);

    /**
     * Возвращает текущую стратегию выделения.
     * 
     * @return 
     *      Текущая стратегия выделения
     */
    alloc_strategy get_alloc_type() const;

    /**
     * Освобождает неиспользуемые чанки, возвращая память обратно операционной системе.
     */
    void release_unused();

    /**
     * Объединяет смежные свободные блоки внутри каждого чанка.
     *
     * Метод проходится по всем чанкам в списке и в каждом из них проверяет,
     * есть ли подряд идущие свободные блоки. Если такие блоки найдены,
     * они объединяются в один более крупный свободный блок.
     *
     * Это снижает фрагментацию внутри чанков и позволяет более эффективно
     * использовать уже выделенную память.
     */
    void merge_free_blocks();

    /**
     * Выделяет блок памяти указанного размера.
     *
     * @param sz 
     *      Размер в байтах
     * 
     * @return 
     *      Указатель на выделенный блок памяти
     */
    void* allocate(std::size_t sz) override;

    /**
     * Выделяет выровненный блок памяти.
     *
     * @param sz 
     *      Размер в байтах
     * 
     * @param align 
     *      Требуемое выравнивание, но в этой реализации игнорируется, внутри используется выравнивание std::max_align_t.
     * 
     * @return 
     *      Указатель на выровненный блок памяти
     */
    void* allocate_align(std::size_t sz, std::size_t align) override;

    /**
     * Освобождает ранее выделенный блок.
     *
     * @note
     *      Правильно работает с nullptr.
     * 
     * @param p 
     *      Указатель на блок
     * 
     * @param sz 
     *      Размер блока
     */
    void deallocate(void* p, std::size_t sz) override;

    /**
     * Выводит лог состояния аллокатора - информация о чанках и распределении памяти.
     */
    void print_log() const;
};


}
#endif//_TCA_ALLOCATORS_SHARED_ALLOCATOR_H