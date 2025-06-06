#include <cpp/lang/utils/images/image_tree.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/unique_ptr.hpp>
#include <new>

namespace jstd {
namespace texturing {

    int rect::to_string(char buf[], int bufsize) const {
        return std::snprintf(buf, bufsize, "[x=%li, y=%li, w=%li, h=%li]", (long int) x, (long int) y, (long int) w, (long int) h);
    }

    node::node() : 
    m_allocator(nullptr), 
    m_left(nullptr), 
    m_right(nullptr), 
    m_rect(), 
    m_ID(NULL_ID) {

    }

    node::node(int32_t w, int32_t h, tca::base_allocator* allocator) : node({0,0,w,h}, allocator) {

    }

    node::node(const rect& r, tca::base_allocator* allocator) : 
    m_allocator(allocator), 
    m_left(nullptr), 
    m_right(nullptr), 
    m_rect(r), 
    m_ID(NULL_ID) {

    }

    node::node(node&& n) :
    m_allocator(n.m_allocator),
    m_left(n.m_left),
    m_right(n.m_right),
    m_rect(n.m_rect),
    m_ID(n.m_ID) {
        n.m_allocator   = nullptr;
        n.m_left        = nullptr;
        n.m_right       = nullptr;
        n.m_ID          = NULL_ID;
    }
    
    node& node::operator= (node&& n) {
        if (&n != this) {
            cleanup();
            m_allocator = n.m_allocator;
            m_left      = n.m_left;
            m_right     = n.m_right;
            m_rect      = n.m_rect;
            m_ID        = n.m_ID;

            n.m_allocator   = nullptr;
            n.m_left        = nullptr;
            n.m_right       = nullptr;
            n.m_ID          = NULL_ID;
        }
        return *this;
    }
    
    void node::cleanup() {
        if (m_allocator != nullptr) {
            node* chields[2] {m_right, m_left};
            for (int i = 0; i < 2; ++i) {
                if (chields[i] != nullptr) {
                    chields[i]->~node();
                    m_allocator->deallocate(chields[i], sizeof(node));
                }
            }
            m_right     = nullptr;
            m_left      = nullptr;
            m_allocator = nullptr;
        }
    }

    node::~node() {
        cleanup();
    }

    bool node::is_leaf() const {
        return m_left == nullptr && m_right == nullptr;
    }

    node* node::put_image(int32_t w, int32_t h, int32_t imageID) {
        
        if (m_allocator == nullptr)
            throw_except<illegal_state_exception>("allocator must be != null");
        
        if (w < 0 || h < 0)
            throw_except<illegal_argument_exception>("Width or height must be >= 0");

        if (!is_leaf()){
			node* n = m_left->put_image(w, h, imageID);
			if (n != nullptr)
				return n;
			return m_right->put_image(w, h, imageID);
		}

        if (m_ID != NULL_ID || m_rect.w < w || m_rect.h < h)
			return nullptr;

        
		if (m_rect.w == w && m_rect.h == h){
			m_ID = imageID;
			return this;
		}
        
        unique_ptr<node> left(m_allocator);
        unique_ptr<node> right(m_allocator);

		if (m_rect.w - w > m_rect.h - h){	//Если высота больше ширины, то разбиваем по x
            *left   = node({m_rect.x, 		m_rect.y, 		w, 				m_rect.h},      m_allocator);
            *right  = node({m_rect.x + w, 	m_rect.y, 		m_rect.w - w, 	m_rect.h},      m_allocator);   
		}
        
        else {								//Иначе по y
			*left   = node({m_rect.x, 		m_rect.y, 		m_rect.w, 		h},             m_allocator);
			*right  = node({m_rect.x, 		m_rect.y + h,	m_rect.w, 		m_rect.h - h},  m_allocator);
		}
        
        m_left  = left.release();
        m_right = right.release();

		return m_left->put_image(w, h, imageID);
    }

    const rect& node::get_rect() const {
        return m_rect;
    }

    int32_t node::get_id() const {
        return m_ID;
    }

    int node::to_string(char buf[], int bufsize) const {
        char rect_str_buffer[64];
        m_rect.to_string(rect_str_buffer, sizeof(rect_str_buffer));
        return std::snprintf(buf, bufsize, "[%s, ID=%li]", rect_str_buffer, (long int) m_ID);
    }

}//namespace jstd
}//namespace texturing