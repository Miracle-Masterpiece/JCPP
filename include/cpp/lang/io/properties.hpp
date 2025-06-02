#ifndef _JSTD_CPP_LANG_IO_PROPERTIES_H_
#define _JSTD_CPP_LANG_IO_PROPERTIES_H_

#include <cpp/lang/utils/hash_map.hpp>
#include <cpp/lang/ustring.hpp>
#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/io/istream.hpp>

namespace jstd {

/**
 * @brief Класс для хранения и управления парой "ключ-значение".
 * 
 * Позволяет устанавливать, получать и загружать свойства из потока ввода/вывода.
 * Использует хеш-таблицу (hash_map<u8string, u8string>) для хранения данных.
 */
class properties {
    //Аллокатор для управления памятью
    tca::base_allocator*            _allocator; 
    //Хеш-таблица для хранения свойств
    hash_map<string, string>    _props;
public:
    
    /**
     * @brief Создает объект свойств с заданным аллокатором.
     * 
     * @param allocator 
     *      Указатель на пользовательский аллокатор.
     */
    properties(tca::base_allocator* allocator = tca::get_scoped_or_default());

    /**
     * @brief Устанавливает значение свойства по ключу.
     * 
     * @param key 
     *      Ключ свойства.
     * 
     * @param value 
     *      Значение свойства.
     */
    void set(const string& key, const string& value);

    /**
     * @brief Устанавливает значение свойства по ключу (перегрузка для `const char*`).
     * 
     * @param key 
     *      Ключ свойства.
     * 
     * @param value 
     *      Значение свойства.
     */
    void set(const char* key, const char* value);

    /**
     * @brief Получает значение свойства по ключу.
     * 
     * @param key 
     *      Ключ свойства.
     * 
     * @return 
     *      Указатель на значение или `nullptr`, если ключ отсутствует.
     */
    const string* get(const string& key) const;

    /**
     * @brief Получает значение свойства по ключу (перегрузка для `const char*`).
     * 
     * @param key 
     *      Ключ свойства.
     * 
     * @return 
     *      Указатель на значение или `nullptr`, если ключ отсутствует.
     */
    const string* get(const char* key) const;

    /**
     * @brief Получает значение свойства или возвращает значение по умолчанию.
     * 
     * @param key 
     *      Ключ свойства.
     * 
     * @param _default 
     *      Значение по умолчанию, если ключ отсутствует.
     * 
     * @return 
     *      Ссылка на значение, если ключ найден, иначе `_default`.
     */
    const string& get_or_default(const string& key, const string& _default) const;

    /**
     * @brief Сохраняет свойства в поток.
     * 
     * @param out 
     *      Поток вывода.
     */
    void save(ostream& out) const;

    /**
     * @brief Загружает свойства из потока.
     * 
     * @param in 
     *      Поток ввода.
     */
    void load(istream& in);

    /**
     * @brief Проверяет, содержит ли объект свойства.
     * 
     * @return 
     *      true, если свойств нет, иначе false.
     */
    bool is_empty() const;

    /**
     * @brief Возвращает итератор на начало контейнера свойств.
     * 
     * @return 
     *      Константный итератор на первый элемент.
     */
    hash_map<string, string>::const_iterator begin() const {
        return _props.begin();
    }

    /**
     * @brief Возвращает итератор на конец контейнера свойств.
     * 
     * @return 
     *      Константный итератор на элемент, следующий за последним.
     */
    hash_map<string, string>::const_iterator end() const {
        return _props.end();
    }
};

}
#endif//_JSTD_CPP_LANG_IO_PROPERTIES_H_