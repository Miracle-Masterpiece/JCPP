#ifndef JSTD_CPP_LANG_UTILS_IMAGES_IMAGE_TREE_H
#define JSTD_CPP_LANG_UTILS_IMAGES_IMAGE_TREE_H

#include <allocators/base_allocator.hpp>
#include <cstdint>

namespace jstd {


template<typename T>
void throw_except(const char*, ...);

namespace texturing {

struct rect {
    int32_t x, y, w, h;
    int to_string(char buf[], int bufsize) const;
};

/**
 * Представляет узел дерева для размещения изображений в атласе текстур.
 *
 * Каждый узел может быть либо листом (занимает прямоугольник),
 * либо внутренним узлом, разбивающим пространство между потомками.
 */
class node {
public:
    /**
     * Константа, обозначающая отсутствие изображения.
     */
    static const int32_t NULL_ID = -1;
private:
    tca::base_allocator* m_allocator; // Аллокатор, используемый для управления памятью узлов.
    node* m_left;  // Левый дочерний узел.
    node* m_right; // Правый дочерний узел.
    rect m_rect;   // Прямоугольник, занимаемый данным узлом.
    int32_t m_ID;  // Идентификатор изображения в этом узле или NULL_ID, если пуст.

    /**
     * 
     */
    node(const node&) = delete;

    /**
     * 
     */
    node& operator= (const node&) = delete;

    /**
     * Рекурсивно очищает потомков и освобождает память.
     */
    void cleanup();

    /**
     * Рекурсивная вспомогательная функция для обхода в глубину.
     * 
     * Обходит переданное поддерево, вызывая контекст для всех узлов,
     * у которых установлен валидный идентификатор изображения.
     * 
     * @tparam CONTEXT 
     *      Класс или объект с перегруженным оператором {@code void operator()(const node*) }
     * 
     * @param context 
     *      Контекст, применяемый к каждому листу, содержащему изображение.
     * 
     * @param n 
     *      Указатель на текущий узел дерева. Если nullptr, обход прекращается.
     */
    template<typename CONTEXT>
    void depth_search0(CONTEXT& contex, node* n) const;

public:
    /**
     * Конструктор по умолчанию.
     * Создаёт пустой узел с NULL_ID и нулевым прямоугольником.
     */
    node();

    /**
     * Создаёт корневой узел заданной ширины и высоты.
     *
     * @param w 
     *      Ширина прямоугольника.
     * 
     * @param h 
     *      Высота прямоугольника.
     * 
     * @param allocator 
     *      Указатель на аллокатор памяти.
     */
    node(int32_t w, int32_t h, tca::base_allocator* allocator);

    /**
     * Создаёт корневой узел с заданным прямоугольником.
     *
     * @param r 
     *      Прямоугольная область.
     * 
     * @param allocator 
     *      Указатель на аллокатор памяти.
     */
    node(const rect& r, tca::base_allocator* allocator);

    /**
     * Перемещающий конструктор.
     *
     * @param other 
     *      Узел, из которого будет перемещено содержимое.
     */
    node(node&& other);

    /**
     * Перемещающий оператор присваивания.
     *
     * @param other 
     *      Узел, из которого будет перемещено содержимое.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    node& operator= (node&& other);

    /**
     * Деструктор. Освобождает ресурсы и рекурсивно очищает потомков.
     */
    ~node();

    /**
     * Проверяет, является ли узел листом (не имеет потомков).
     *
     * @return 
     *      true, если узел — лист, иначе false, если содержит дочерние узлы.
     */
    bool is_leaf() const;

    /**
     * Пытается разместить изображение заданных размеров в этом узле.
     * Если размещение успешно — возвращает указатель на занятый узел.
     *
     * @param w 
     *      Ширина изображения.
     * 
     * @param h 
     *      Высота изображения.
     * 
     * @param imageID 
     *      Идентификатор изображения.
     * 
     * @return 
     *      Указатель на узел, в который помещено изображение, или nullptr, если не удалось.
     */
    node* put_image(int32_t w, int32_t h, int32_t imageID);

    /**
     * Возвращает строковое представление узла.
     * 
     * @param buf
     *      Буфер в который будет сохранена строка.
     * 
     * @param bufsize
     *      Размер буфера.
     * 
     * @return
     *      Кол-во записанных символов.
     */
    int to_string(char buf[], int bufsize) const;

    /**
     * Выполняет обход в глубину узлов дерева, начиная с текущего узла.
     * 
     * Функция вызывает переданный контекст (callable объект) для каждого листа, 
     * у которого установлен идентификатор изображения (m_ID != NULL_ID).
     * 
     * @tparam CONTEXT 
     *      Класс или объект с перегруженным оператором {@code void operator()(const node*) }
     * 
     * @param context 
     *      Контекст, вызываемый для каждого подходящего узла.
     */
    template<typename CONTEXT>
    void depth_search(CONTEXT& contex) const;

    /**
     * Возвращает прямоугольник, который описывает расположение и размер этого узла.
     */
    const rect& get_rect() const;

    /**
     * Возвращает идентификатор узла.
     */
    int32_t get_id() const;
};

    template<typename CONTEXT>
    void node::depth_search0(CONTEXT& context, node* n) const {
        if (n == nullptr)
            return;
        
        if (n->m_ID != NULL_ID)
            context(const_cast<const node*>(n));
        
        depth_search0(context, n->m_left);
        depth_search0(context, n->m_right);
    }

    template<typename CONTEXT>
    void node::depth_search(CONTEXT& context) const {
        depth_search0(context, m_left);
        depth_search0(context, m_right);
    }

}//namespace jstd
}//namespace texturing 




#endif//JSTD_CPP_LANG_UTILS_IMAGES_IMAGE_TREE_H