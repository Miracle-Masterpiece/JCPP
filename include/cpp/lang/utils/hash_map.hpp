#ifndef JSTD_CPP_LANG_HASH_MAP_H_
#define JSTD_CPP_LANG_HASH_MAP_H_

#include <allocators/base_allocator.hpp>
#include <cstdint>
#include <utility>
#include <cpp/lang/array.hpp>
#include <cpp/lang/utils/hash.hpp>
#include <cassert>
#include <iostream>

namespace jstd {

/**
 * @brief Узел, используемый в `hash_map`.
 * 
 * @tparam K Тип ключа.
 * @tparam V Тип значения.
 * 
 * @details
 *      Узлы связаны в цепочку (Linked List) для обработки коллизий.
 */
template<typename K, typename V>
class map_node {
    map_node(const map_node<K, V>& node) = delete;
    map_node<K, V>& operator= (const map_node<K, V>& node) = delete;
    
    uint64_t        _hashcode;  ///< Хэш-код ключа.
    map_node<K, V>* _next;      ///< Следующий узел в цепочке (коллизия).
    K               _key;       ///< Ключ.
    V               _value;     ///< Значение.

public:
    /**
     * @brief Конструктор по умолчанию.
     * 
     * @details Создаёт пустой узел с нулевым хэшем и пустыми значениями.
     */
    map_node();

    /**
     * @brief Конструктор с параметрами.
     * 
     * @tparam _K Тип передаваемого ключа.
     * @tparam _V Тип передаваемого значения.
     * @param key Ключ.
     * @param value Значение.
     * @param hashcode Хэш-код ключа.
     */
    template<typename _K, typename _V>
    map_node(_K&& key, _V&& value, uint64_t hashcode);

    /**
     * @brief Конструктор перемещения.
     * 
     * @param node Перемещаемый узел.
     */
    map_node(map_node<K, V>&& node);

    /**
     * @brief Оператор перемещения.
     * 
     * @param node Перемещаемый узел.
     * @return map_node<K, V>& Ссылка на текущий объект.
     */
    map_node<K, V>& operator= (map_node<K, V>&& node);

    //
    ~map_node();

    /**
     * @brief Устанавливает ключ узла.
     * 
     * @tparam _K Тип передаваемого ключа.
     * @param key Новый ключ.
     */
    template<typename _K>
    void set_key(_K&& key) {
        _key = std::forward<_K>(key);
    }

    /**
     * @brief Устанавливает значение узла.
     * 
     * @tparam _V Тип передаваемого значения.
     * @param value Новое значение.
     */
    template<typename _V>
    void set_value(_V&& value) {
        _value = std::forward<_V>(value);
    }

    /**
     * @brief Возвращает ключ узла.
     * 
     * @return const K& Ключ.
     */
    const K& get_key() const {
        return _key;
    }

    /**
     * @brief Возвращает значение узла.
     * 
     * @return V& Значение.
     */
    V& get_value() {
        return _value;
    }

    /**
     * @brief Возвращает значение узла (константная версия).
     * 
     * @return const V& Значение.
     */
    const V& get_value() const {
        return _value;
    }

    /**
     * @brief Возвращает хэш-код ключа.
     * 
     * @return uint64_t Хэш-код.
     */
    uint64_t get_hashcode() const {
        return _hashcode;
    }

    /**
     * @brief Устанавливает следующий узел в цепочке.
     * 
     * @param next Указатель на следующий узел.
     */
    void set_next(map_node<K, V>* next) {
        _next = next;
    }

    /**
     * @brief Возвращает указатель на следующий узел (константная версия).
     * 
     * @return const map_node<K, V>* Указатель на следующий узел.
     */
    const map_node<K, V>* get_next() const {
        return _next;
    }

    /**
     * @brief Возвращает указатель на следующий узел.
     * 
     * @return map_node<K, V>* Указатель на следующий узел.
     */
    map_node<K, V>* get_next() {
        return _next;
    }
};

    template<typename K, typename V>
    map_node<K, V>::map_node() : _hashcode(0), _next(nullptr), _key(), _value() {
        
    }

    template<typename K, typename V>
    template<typename _K, typename _V>
    map_node<K, V>::map_node(_K&& key, _V&& value, uint64_t hashcode) : 
    _hashcode(hashcode), 
    _next(nullptr),
    _key(std::forward<_K>(key)), 
    _value(std::forward<_V>(value)) {

    }

    template<typename K, typename V>
    map_node<K, V>::map_node(map_node<K, V>&& node) : 
    _hashcode(node._hashcode), 
    _next(node._next),
    _key(std::move(node._key)), 
    _value(std::move(node._value)) {
        node._hashcode  = 0;
        node._next      = nullptr;
    }
    
    template<typename K, typename V>
    map_node<K, V>& map_node<K, V>::operator= (map_node<K, V>&& node) {
        if (&node != this) {
            _hashcode   = node._hashcode;
            _next       = node._next;
            _key        = std::move(node._key);
            _value      = std::move(node._value);

            node._hashcode  = 0;
            node._next      = nullptr;
        }
        return *this;
    }

    template<typename K, typename V>
    map_node<K, V>::~map_node() {

    }

template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
class hash_map_entry_interator;

/**
 * @class hash_map
 * 
 * @brief Шаблонная хеш-таблица с разрешением коллизий методом цепочек.
 *
 * Реализация предоставляет эффективные операции вставки, удаления и поиска элементов
 * по ключу. При превышении порога заполненности (`load factor`) выполняется автоматическая
 * рехешировка с увеличением количества корзин.
 *
 * @tparam K Тип ключа.
 * @tparam V Тип значения.
 * @tparam KEY_HASH Функтор для вычисления хеша (по умолчанию `hash_for<K>`).
 * @tparam KEY_EQUAL Функтор для сравнения ключей (по умолчанию `equal_to<K>`).
 */
template<typename K, typename V, typename KEY_HASH = hash_for<K>, typename KEY_EQUAL = equal_to<K>>
class hash_map {
    static const int DEFAULT_CAPACITY = 16;
    
    tca::base_allocator*    _allocator;
    array<map_node<K, V>*>  _buckets;
    int64_t                 _size;
    float                   _loadfactor;

    void rehash();
    void put_node(map_node<K, V>* node);

    template<typename _K, typename _V>
    map_node<K, V>* new_node(_K&&, _V&&, int64_t hashcode);
    
    void delete_node(map_node<K, V>* node);

public:
    /**
     * @brief Конструктор по умолчанию.
     *  Создаёт пустую хеш-таблицу.
     */
    hash_map();

    /**
     * @brief Создаёт hash_map с указанным аллокатором, ёмкостью и коэффициентом загрузки.
     * 
     * @param allocator    
     *      Пользовательский аллокатор памяти.
     * 
     * @param init_capacity 
     *      Начальная вместимость (по умолчанию DEFAULT_CAPACITY).
     * 
     * @param loadfactor   
     *      Коэффициент загрузки (по умолчанию 0.75).
     */
    hash_map(tca::base_allocator* allocator, int64_t init_capacity = DEFAULT_CAPACITY, float loadfactor = 0.75f);
    
    /**
     * @brief Конструктор копирования.
     * 
     * @param map hash_map 
     *      Хеш-карта которую необходимо скопировать.
     * 
     * @note
     *      В копии-объекта будет содержаться аллокатор из копируемого объекта.
     */
    hash_map(const hash_map<K, V, KEY_HASH, KEY_EQUAL>& map);
    
    /**
     * @brief Конструктор перемещения.
     * 
     * @param map 
     *      Хеш-карта, которая будет перемещена в этот объект.
     */
    hash_map(hash_map<K, V, KEY_HASH, KEY_EQUAL>&& map);
    
    /**
     * @brief оператор копирования.
     * 
     * @param map hash_map 
     *      Хеш-карта которую необходимо скопировать.
     * 
     * @note
     *      В копии-объекта будет содержаться аллокатор из копируемого объекта.
     */
    hash_map<K, V, KEY_HASH, KEY_EQUAL>& operator= (const hash_map<K, V, KEY_HASH, KEY_EQUAL>& map);
    
    /**
     * @brief Оператор перемещения.
     * 
     * @param map 
     *      Хеш-карта, которая будет перемещена в этот объект.
     * 
     * @note
     *      Если внутри этой карты уже есть ненулевой аллокатор, то при копировании будет использоваться он.
     *      Если внутри этой карты аллокатор имеет значение null, то аллокатор используется из передаваемой карты.
     */
    hash_map<K, V, KEY_HASH, KEY_EQUAL>& operator= (hash_map<K, V, KEY_HASH, KEY_EQUAL>&& map);
    
    /**
     * @brief очищает память.
     */
    ~hash_map();

    /**
     * @brief Добавляет новый ключ-значение в хеш-таблицу.
     * 
     * @tparam _K Тип ключа.
     * @tparam _V Тип значения.
     * @param key   Ключ (копируется или перемещается).
     * @param value Значение (копируется или перемещается).
     * 
     * @return 
     *      true - eсли значение было добавлено, иначе false.
     */
    template<typename _K, typename _V>
    bool put(_K&& key, _V&& value);
    
    /**
     * Заменяет значение по ключу.
     * 
     * @tparam K 
     *      Тип ключа.
     * 
     * @tparam _V 
     *      Тип значения.
     * 
     * @param key 
     *      Ключ по которому осуществляется поиск.
     * 
     * @param new_value 
     *      Значение (копируется или перемещается).
     * 
     * @param old_value (Оптионально)
     *      Если указатель не равер null, то туда запишется старое значение.
     * 
     @return 
     *      true - eсли значение было изменено, иначе false.
     */
    template<typename _V>
    bool replace(const K& key, _V&& new_value, V* old_value = nullptr);

    /**
     * @brief Получает указатель на значение по ключу.
     * 
     * @param key 
     *      Ключ, по которому ищется значение.
     * 
     * @return 
     *      Указатель на значение, либо nullptr, если ключ не найден.
     */
    V* get(const K& key);
    
    /**
     * @brief Получает указатель на значение по ключу (константная версия).
     * 
     * @param key 
     *      Ключ, по которому ищется значение.
     * 
     * @return 
     *      Указатель на значение, либо `nullptr`, если ключ не найден.
     */
    const V* get(const K& key) const;

    /**
     * @brief Получает значение по ключу или возвращает значение по умолчанию.
     * 
     * @param key      
     *      Ключ, по которому ищется значение.
     * 
     * @param _default 
     *      Значение по умолчанию, если ключ не найден.
     * 
     * @return 
     *      Ссылка на найденное или переданное значение по умолчанию.
     */
    V& get_or_default(const K& key, V& _default);

    /**
     * @brief Получает значение по ключу или возвращает значение по умолчанию (константная версия).
     * 
     * @param key      
     *      Ключ, по которому ищется значение.
     * 
     * @param _default 
     *      Значение по умолчанию, если ключ не найден.
     * 
     * @return 
     *      Ссылка на найденное или переданное значение по умолчанию.
     */
    const V& get_or_default(const K& key, const V& _default) const;

    /**
     * @brief Проверяет, содержится ли ключ в хеш-таблице.
     * 
     * @param key 
     *      Ключ для поиска.
     * 
     * @return 
     *      true, если ключ найден, иначе false.
     */
    bool contains(const K& key) const;

    bool contains_all(const hash_map<K, V, KEY_HASH, KEY_EQUAL>& map) const;

    /**
     * @brief Удаляет элемент по ключу.
     * 
     * @param key 
     *      Ключ элемента, который требуется удалить.
     * 
     * @param _return 
     *      Если передан указатель, в него будет записано удалённое значение.
     * 
     * @return 
     *      true, если удаление прошло успешно, иначе false.
     */
    bool remove(const K& key, V* _return = nullptr);

    /**
     * @brief Возвращает текущее количество элементов в хеш-таблице.
     * 
     * @return 
     *      Число элементов.
     */
    int64_t size() const;

    /**
     * @brief Проверяет, пуста ли хеш-таблица.
     * 
     * @return 
     *      true, если таблица пуста, иначе false.
     */
    bool is_empty() const;

    /**
     * @brief 
     *      Очищает хеш-таблицу, удаляя все элементы.
     */
    void clear();

    /**
     * @brief Создаёт клон текущей hash_map.
     * 
     * @param allocator 
     *      Пользовательский аллокатор (по умолчанию используется текущий).
     * 
     * @return 
     *      Новый объект hash_map, идентичный текущему.
     */
    hash_map<K, V, KEY_HASH, KEY_EQUAL> clone(tca::base_allocator* allocator = nullptr) const;

    /**
     * Выводит список ключ-значение карты.
     * 
     * @remark
     *      У хранимых значений должен быть определён оператор вывода 
     *                                                                  std::ostream& operator << (std::ostream& out, const T&);
     */
    void print() const;

    /**
     * Добавляет пары ключ-значение из передаваемой хеш-карты в эту хеш-карту.
     */
    void put_all(const hash_map<K, V, KEY_HASH, KEY_EQUAL>&);

    /**
     * Константный итератор для перебора [ключ-значение]
     */
    class const_iterator {
        const hash_map<K, V, KEY_HASH, KEY_EQUAL>* _map;
        const map_node<K, V>*                      _node;
        int64_t                                    _idx;
        void next_node();
    public:
        const_iterator();   
        
        const_iterator(const const_iterator&);    
        const_iterator(const_iterator&&);    

        const_iterator& operator= (const const_iterator&);    
        const_iterator& operator= (const_iterator&&);    

        ~const_iterator();

        const_iterator(const hash_map<K, V, KEY_HASH, KEY_EQUAL>* map);
        bool operator != (const const_iterator& it) const;
        const_iterator& operator++();
        const_iterator operator++(int);
        const map_node<K, V>& operator*() const;
    };

    /**
     * Возвращает итератор на начало хеш-карты.
     */
    const_iterator begin() const;
    
    /**
     * Возвращает итератор на конец хеш-карты.
     */
    const_iterator end() const;
};

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    hash_map<K, V, KEY_HASH, KEY_EQUAL>::hash_map() : _allocator(nullptr), _buckets(), _size(0), _loadfactor(0) {
        
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    hash_map<K, V, KEY_HASH, KEY_EQUAL>::hash_map(const hash_map<K, V, KEY_HASH, KEY_EQUAL>& map) : hash_map<K, V, KEY_HASH, KEY_EQUAL>() {
        *this = std::move(map.clone());
    }
    
    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    hash_map<K, V, KEY_HASH, KEY_EQUAL>::hash_map(hash_map<K, V, KEY_HASH, KEY_EQUAL>&& map) : 
    _allocator(map._allocator),
    _buckets(std::move(map._buckets)),
    _size(map._size),
    _loadfactor(map._loadfactor) {
        map._allocator  = nullptr;
        map._size       = 0;
        map._loadfactor = 0;
    }
    
    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    hash_map<K, V, KEY_HASH, KEY_EQUAL>& hash_map<K, V, KEY_HASH, KEY_EQUAL>::operator= (const hash_map<K, V, KEY_HASH, KEY_EQUAL>& map) {
        if (&map != this)
            this->operator=(std::move(map.clone(_allocator)));
        return *this;
    }
    
    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    hash_map<K, V, KEY_HASH, KEY_EQUAL>& hash_map<K, V, KEY_HASH, KEY_EQUAL>::operator= (hash_map<K, V, KEY_HASH, KEY_EQUAL>&& map) {
        if (&map != this) {
            clear();
            _allocator  = map._allocator;
            _buckets    = std::move(map._buckets);
            _size       = map._size;
            _loadfactor = map._loadfactor;

            map._size       = 0;
            map._loadfactor = 0;
        }
        return *this;
    }


    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    hash_map<K, V, KEY_HASH, KEY_EQUAL>::hash_map(tca::base_allocator* allocator, int64_t init_capacity, float loadfactor) :
    _allocator(allocator),
    _buckets(allocator, init_capacity),
    _size(0),
    _loadfactor(loadfactor) {
        _buckets.set(nullptr);
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    template<typename _K, typename _V>
    map_node<K, V>* hash_map<K, V, KEY_HASH, KEY_EQUAL>::new_node(_K&& key, _V&& value, int64_t hashcode) {
        void* mem = _allocator->allocate_align(sizeof(map_node<K, V>), alignof(map_node<K, V>));
        if (mem == nullptr)
            throw_except<out_of_memory_error>("Out of memory");
        return new(mem) map_node<K, V>(std::forward<_K>(key), std::forward<_V>(value), hashcode);
    }
    
    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    void hash_map<K, V, KEY_HASH, KEY_EQUAL>::delete_node(map_node<K, V>* node) {
        assert(node != nullptr);
        node->~map_node();
        _allocator->deallocate(node, sizeof(map_node<K, V>));
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    template<typename _V>
    bool hash_map<K, V, KEY_HASH, KEY_EQUAL>::replace(const K& key, _V&& new_value, V* old_value) {
        if (size() == 0)
            return false;
        KEY_HASH hasher;
        KEY_EQUAL is_equal;
        
        uint64_t hashcode   = hasher(key);
        uint64_t index      = hashcode % _buckets.length;

        for (map_node<K, V>* n = _buckets[index]; n != nullptr; n = n->get_next()) {
            if (is_equal(key, n->get_key())) {
                if (old_value != nullptr)
                    (*old_value) = std::move(n->get_value());
                n->set_value(std::forward<_V>(new_value));
                return true;
            }
        }

        return false;
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    template<typename _K, typename _V>
    bool hash_map<K, V, KEY_HASH, KEY_EQUAL>::put(_K&& key, _V&& value) {
        if (_buckets.length == 0) {
            if (_allocator == nullptr)
                throw_except<illegal_state_exception>("allocator must be != null");
            _buckets = array<map_node<K, V>*>(_allocator, DEFAULT_CAPACITY);
            _buckets.set(nullptr);
        }
        
        if ((double)_size / (double)_buckets.length > _loadfactor)
            rehash();

        KEY_HASH hasher;
        uint64_t hashcode   = hasher(key);
        uint64_t index      = hashcode % _buckets.length;
        if (_buckets[index] == nullptr) {
            _buckets[index] = new_node(std::forward<_K>(key), std::forward<_V>(value), hashcode);
        } else {
            typedef map_node<K, V> map_node;
            assert(_buckets[index] != nullptr);
            
            KEY_EQUAL is_equal;
            map_node* prev = nullptr;
            for (map_node* i = _buckets[index]; i != nullptr; prev = i, i = i->get_next()) {
                assert(i != nullptr);
                if (is_equal(i->get_key(), key))
                    return false;
            }

            assert(prev != nullptr);
            map_node* _new = new_node(std::forward<_K>(key), std::forward<_V>(value), hashcode);
            prev->set_next(_new);
        }
        ++_size;
        return true;
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    void hash_map<K, V, KEY_HASH, KEY_EQUAL>::put_all(const hash_map<K, V, KEY_HASH, KEY_EQUAL>& map) {
        for (const map_node<K, V>& entry : map)
            put(entry.get_key(), entry.get_value());
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    typename hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator hash_map<K, V, KEY_HASH, KEY_EQUAL>::begin() const {
        return const_iterator(this);
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    typename hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator hash_map<K, V, KEY_HASH, KEY_EQUAL>::end() const {
        return const_iterator();
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_map<K, V, KEY_HASH, KEY_EQUAL>::contains(const K& key) const {
        if (_size == 0)
            return false;
        
        KEY_HASH hasher;
        KEY_EQUAL is_equal;

        uint64_t hashcode   = hasher(key);
        uint64_t index      = hashcode % _buckets.length;
        for (const map_node<K, V>* n = _buckets[index]; n != nullptr; n = n->get_next()) {
            if (is_equal(key, n->get_key()))
                return true;
        }
        
        return false;
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_map<K, V, KEY_HASH, KEY_EQUAL>::contains_all(const hash_map<K, V, KEY_HASH, KEY_EQUAL>& map) const {
        if (map.size() != size())
            return false;
        for (const map_node<K, V>& entry : map) {
            if (!contains(entry.get_key()))
                return false;
        }
        return true;
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    V& hash_map<K, V, KEY_HASH, KEY_EQUAL>::get_or_default(const K& key, V& _default) {
        V* result = get(key);
        if (result != nullptr)
            return  *result;
        return _default;   
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    const V& hash_map<K, V, KEY_HASH, KEY_EQUAL>::get_or_default(const K& key, const V& _default) const {
        const V* result = get(key);
        if (result != nullptr)
            return  *result;
        return _default;   
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    V* hash_map<K, V, KEY_HASH, KEY_EQUAL>::get(const K& key) {
        if (_size == 0)
            return nullptr;
        KEY_HASH hasher;
        KEY_EQUAL is_equal;
        uint64_t hashcode   = hasher(key);
        uint64_t index      = hashcode % _buckets.length;
        for (map_node<K, V>* n = _buckets[index]; n != nullptr; n = n->get_next()) {
            if (is_equal(key, n->get_key()))
                return &n->get_value();
        }
        return nullptr;
    }
    
    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    const V* hash_map<K, V, KEY_HASH, KEY_EQUAL>::get(const K& key) const {
        hash_map<K, V>* _this = const_cast<hash_map*>(this);
        return _this->get(key);
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_map<K, V, KEY_HASH, KEY_EQUAL>::remove(const K& key, V* _return) {
        if (_size == 0)
            return false;
        
        KEY_HASH hasher;
        KEY_EQUAL is_equal;
        uint64_t hashcode   = hasher(key);
        uint64_t index      = hashcode % _buckets.length;
        
        for (map_node<K, V>* node = _buckets[index], *prev = nullptr; node != nullptr; prev = node, node = node->get_next()) {
            if (is_equal(key, node->get_key())) {
                
                if (prev == nullptr) {
                    _buckets[index] = node->get_next();
                } else {
                    assert(prev != nullptr);
                    prev->set_next(node->get_next());
                }
                
                if (_return != nullptr)
                    *_return = std::move(node->get_value());
                
                delete_node(node);
                
                --_size;
                return true;
            }
        }
        return false;
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    hash_map<K, V, KEY_HASH, KEY_EQUAL>::~hash_map() {
        clear();
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_map<K, V, KEY_HASH, KEY_EQUAL>::is_empty() const {
        return _size == 0;
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    void hash_map<K, V, KEY_HASH, KEY_EQUAL>::print() const {
        for (int64_t i = 0; i < _buckets.length; ++i) {
            const map_node<K, V>* node = _buckets[i];
            if (node != nullptr) {
                for (const map_node<K, V>* n = node; n != nullptr; n = n->get_next()) {
                    std::cout << "[" << n->get_key() << '=' << n->get_value() << "]\n";
                }
            }
        }
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    int64_t hash_map<K, V, KEY_HASH, KEY_EQUAL>::size() const {
        return _size;
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    void hash_map<K, V, KEY_HASH, KEY_EQUAL>::clear() {
        for (int64_t i = 0; i < _buckets.length; ++i) {
            map_node<K, V>* node = _buckets[i];
            if (node != nullptr) {
                for (map_node<K, V>* n = node; n != nullptr;) {
                    map_node<K, V>* for_delete = n;
                    n = n->get_next();
                    delete_node(for_delete);
                }
                _buckets[i] = nullptr;
            }
        }
        _size = 0;
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    void hash_map<K, V, KEY_HASH, KEY_EQUAL>::rehash() {
        array<map_node<K, V>*> old_array;
        
        assert(_buckets.length > 0);
        array<map_node<K, V>*> new_array = array<map_node<K, V>*>(_allocator, _buckets.length * 1.5);    
        new_array.set(nullptr);
        old_array   = std::move(_buckets);
        _buckets    = std::move(new_array);
        
        for (int64_t i = 0; i < old_array.length; ++i) {
            map_node<K, V>* node = old_array[i];
            if (node != nullptr) {
                for (map_node<K, V>* n = node; n != nullptr; ) {
                    map_node<K, V>* current = n;
                    n = n->get_next();
                    put_node(current);
                }
            }
        }
    }
    
    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    void hash_map<K, V, KEY_HASH, KEY_EQUAL>::put_node(map_node<K, V>* node) {
        assert(node != nullptr);
        node->set_next(nullptr);
        uint64_t index = node->get_hashcode() % _buckets.length;
        if (_buckets[index] == nullptr) {
            _buckets[index] = node;
        } else {
            node->set_next(_buckets[index]);
            _buckets[index] = node;
        }
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    hash_map<K, V, KEY_HASH, KEY_EQUAL> hash_map<K, V, KEY_HASH, KEY_EQUAL>::clone(tca::base_allocator* allocator) const {
        
        if (_size == 0) 
            return hash_map<K, V, KEY_HASH, KEY_EQUAL>();
            
        if (allocator == nullptr) {
            if (_allocator == nullptr)
                return hash_map<K, V, KEY_HASH, KEY_EQUAL>();
            allocator = _allocator;
        }
        
        hash_map<K, V, KEY_HASH, KEY_EQUAL> _clone(allocator);
        for (int64_t i = 0; i < _buckets.length; ++i) {
            map_node<K, V>* node = _buckets[i];
            if (node != nullptr) {
                for (map_node<K, V>* n = node; n != nullptr; n = n->get_next())
                    _clone.put(n->get_key(), n->get_value());
            }
        }
        
        return hash_map<K, V, KEY_HASH, KEY_EQUAL>(std::move(_clone));
    }



    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator::const_iterator() :
    _map(nullptr),
    _node(nullptr),
    _idx(0)
    {

    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator::const_iterator(const const_iterator& it) : 
    _map(it._map),
    _node(it._node),
    _idx(it._idx)
    {

    }
    
    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator::const_iterator(const_iterator&& it) {
        _map    = it._map;
        _node   = it._node;
        _idx    = it._idx;

        it._map     = nullptr;
        it._node    = nullptr;
        it._idx     = 0;
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    typename hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator& hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator::operator= (const const_iterator& it) {
        if (&it != this) {
            _map    = it._map;
            _node   = it._node;
            _idx    = it._idx;
        }
        return *this;
    }
    
    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    typename hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator& hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator::operator= (const_iterator&& it) {
        if (&it != this) {
            _map    = it._map;
            _node   = it._node;
            _idx    = it._idx;
    
            it._map     = nullptr;
            it._node    = nullptr;
            it._idx     = 0;
        }
        return *this;
    }
    
    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator::const_iterator(const hash_map<K, V, KEY_HASH, KEY_EQUAL>* map) :
    _map(map),
    _node(nullptr),
    _idx(0) {
        next_node();
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    void hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator::next_node() {
        if ((_node == nullptr || _node->get_next() == nullptr)) {
            for (int64_t i = _idx; i < _map->_buckets.length; ++i) {
                if (_map->_buckets[i] == nullptr)
                    continue;
                _node   = _map->_buckets[i];
                _idx    = i + 1;
                return;
            }
            _node = nullptr;
        } else {
            _node = _node->get_next();
        }
    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator::operator != (const const_iterator& it) const {
        return _node != it._node;
    }
    
    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    typename hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator& hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator::operator++() {
        next_node();
        return *this;
    }
    
    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    typename hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator::operator++(int) {
        const_iterator it = *this;
        next_node();
        return it;
    }
    
    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator::~const_iterator() {

    }

    template<typename K, typename V, typename KEY_HASH, typename KEY_EQUAL>
    const map_node<K, V>& hash_map<K, V, KEY_HASH, KEY_EQUAL>::const_iterator::operator*() const {
        if (_node == nullptr)
            throw_except<null_pointer_exception>("_node must be != null");
        return *_node;
    }

    template class hash_map<int, int>;

}
#endif//JSTD_CPP_LANG_HASH_MAP_H_