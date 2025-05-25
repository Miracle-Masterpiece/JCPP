#ifndef JSTD_CPP_LANG_UTILS_HASH_SET_H
#define JSTD_CPP_LANG_UTILS_HASH_SET_H

#include <cpp/lang/utils/hash_map.hpp>

namespace jstd {

/**
 * Представляет собой множество уникальных элементов на основе хеш-таблицы.
 *
 * Класс hash_set реализован поверх hash_map, где значения элементов не играют роли
 * (используется значение по умолчанию '\0'. 
 * 
 * Предназначен для эффективного хранения и поиска уникальных ключей. 
 * Поддерживает пользовательские хеш-функции и функции сравнения.
 *
 * @tparam K 
 *      Тип ключей.
 * 
 * @tparam KEY_HASH 
 *      Тип хеш-функции для ключей (по умолчанию hash_for<K>).
 * 
 * @tparam KEY_EQUAL 
 *      Тип функции сравнения ключей (по умолчанию equal_to<K>).
 */
template<typename K, typename KEY_HASH = hash_for<K>, typename KEY_EQUAL = equal_to<K>>
class hash_set {
    static const int32_t INIT_CAPACITY  = 16;
    static const char DEFAULT_VALUE     = '\0';
    hash_map<K, char, KEY_HASH, KEY_EQUAL> m_storage;
public:
    /**
     * Создаёт пустое множество.
     */
    hash_set();

    /**
     * Создаёт пустое множество с заданным аллокатором и (опционально) начальной ёмкостью.
     *
     * @param allocator 
     *      Указатель на пользовательский аллокатор.
     * 
     * @param init_capacity 
     *      Начальная ёмкость.
     */
    hash_set(tca::base_allocator* allocator, int64_t init_capacity = INIT_CAPACITY);

    /**
     * Конструктор копирования.
     *
     * @param other 
     *      Множество, из которого будет скопировано содержимое.
     */
    hash_set(const hash_set<K, KEY_HASH, KEY_EQUAL>& other);

    /**
     * Конструктор перемещения.
     *
     * @param other 
     *      Множество, содержимое которого будет перемещено.
     */
    hash_set(hash_set<K, KEY_HASH, KEY_EQUAL>&& other);

    /**
     * Оператор копирующего присваивания.
     *
     * @param other 
     *      Множество, содержимое которого будет скопировано.
     * 
     * @return 
     *      Ссылка на текущее множество.
     */
    hash_set<K, KEY_HASH, KEY_EQUAL>& operator=(const hash_set<K, KEY_HASH, KEY_EQUAL>& other);

    /**
     * Оператор перемещающего присваивания.
     *
     * @param other 
     *      Множество, содержимое которого будет перемещено.
     * 
     * @return 
     *      Ссылка на текущее множество.
     */
    hash_set<K, KEY_HASH, KEY_EQUAL>& operator=(hash_set<K, KEY_HASH, KEY_EQUAL>&& other);

    /**
     * Деструктор.
     */
    ~hash_set();

    /**
     * Добавляет ключ в множество.
     *
     * Если такой ключ уже есть, множество не изменяется.</p>
     *
     * @tparam _K 
     *      Тип передаваемого ключа (поддерживаются rvalue и lvalue).
     * 
     * @param key 
     *      Ключ для добавления.
     */
    template<typename _K>
    void put(_K&& key);

    /**
     * Удаляет ключ из множества.
     *
     * @param key 
     *      Ключ, который требуется удалить.
     * @return 
     *      true, если ключ был найден и удалён, иначе false.
     */
    bool remove(const K& key);

    /**
     * Проверяет, содержится ли ключ в множестве.
     *
     * @param key 
     *      Проверяемый ключ.
     * 
     * @return 
     *      true, если ключ найден, иначе false.
     */
    bool contains(const K& key) const;

    /**
     * Проверяет, содержатся ли все элементы переданного множества в текущем.
     *
     * @param set 
     *      Множество, элементы которого нужно проверить.
     * 
     * @return 
     *      true, если все ключи присутствуют, иначе false.
     */
    bool contains_all(const hash_set<K, KEY_HASH, KEY_EQUAL>& set) const;

    /**
     * Добавляет в текущее множество все элементы из другого множества.
     *
     * @param set 
     *      Множество, ключи которого будут добавлены.
     */
    void put_all(const hash_set<K, KEY_HASH, KEY_EQUAL>& set);

    /**
     * Удаляет все элементы из множества.
     */
    void clear();

    /**
     * Возвращает количество элементов в множестве.
     *
     * @return 
     *      Текущее количество элементов.
     */
    int64_t size() const;

    /**
     * Проверяет, пусто ли множество.
     *
     * @return 
     *      true, если множество не содержит ни одного элемента.
     */
    bool is_empty() const;

    /**
     * Создаёт полную копию множества.
     *
     * @param allocator (необязательно) 
     *      Аллокатор, который будет использоваться для нового множества.
     *      Если аллокатор не указан, будет использоваться текущий.
     * 
     * @return 
     *      Новый экземпляр множества, содержащий те же элементы.
     */
    hash_set<K, KEY_HASH, KEY_EQUAL> clone(tca::base_allocator* allocator = nullptr) const;
};


    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>::hash_set() : m_storage() {

    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>::hash_set(tca::base_allocator* allocator, int64_t init_capacity) : m_storage(allocator, init_capacity) {

    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>::hash_set(const hash_set<K, KEY_HASH, KEY_EQUAL>& set) : m_storage(set.m_storage) {

    }
    
    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>::hash_set(hash_set<K, KEY_HASH, KEY_EQUAL>&& set) : m_storage(std::move(set.m_storage)) {

    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>& hash_set<K, KEY_HASH, KEY_EQUAL>::operator= (const hash_set<K, KEY_HASH, KEY_EQUAL>& set) {
        m_storage = set.m_storage;
        return *this;
    }
    
    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>& hash_set<K, KEY_HASH, KEY_EQUAL>::operator= (hash_set<K, KEY_HASH, KEY_EQUAL>&& set) {
        m_storage = std::move(set.m_storage);
        return *this;
    }
    
    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>::~hash_set() {

    }
    
    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    template<typename _K>
    void hash_set<K, KEY_HASH, KEY_EQUAL>::put(_K&& key) {
        const char value = DEFAULT_VALUE;
        m_storage.put(std::forward<_K>(key), value);
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_set<K, KEY_HASH, KEY_EQUAL>::remove(const K& key) {
        return m_storage.remove(key, nullptr);
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_set<K, KEY_HASH, KEY_EQUAL>::contains(const K& key) const {
        return m_storage.contains(key);
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    void hash_set<K, KEY_HASH, KEY_EQUAL>::put_all(const hash_set<K, KEY_HASH, KEY_EQUAL>& set) {
        m_storage.put_all(set.m_storage);
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    void hash_set<K, KEY_HASH, KEY_EQUAL>::clear() {
        m_storage.clear();
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    int64_t hash_set<K, KEY_HASH, KEY_EQUAL>::size() const {
        return m_storage.size();
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_set<K, KEY_HASH, KEY_EQUAL>::is_empty() const {
        return m_storage.is_empty();
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_set<K, KEY_HASH, KEY_EQUAL>::contains_all(const hash_set<K, KEY_HASH, KEY_EQUAL>& set) const {
        return m_storage.contains_all(set.m_storage);
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL> hash_set<K, KEY_HASH, KEY_EQUAL>::clone(tca::base_allocator* allocator) const {
        hash_map<K, char, KEY_HASH, KEY_EQUAL> storage = m_storage.clone(allocator);
        hash_set<K, KEY_HASH, KEY_EQUAL> result;
        result.m_storage = std::move(storage);
        return hash_set<K, KEY_HASH, KEY_EQUAL>(std::move(result));
    }

}
#endif//JSTD_CPP_LANG_UTILS_HASH_SET_H
