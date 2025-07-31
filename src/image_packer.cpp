#include <allocators/linear_allocator.hpp>
#include <cpp/lang/utils/images/image_packer.hpp>
#include <cpp/lang/utils/images/image_tree.hpp>
#include <cpp/lang/utils/images/image.hpp>
#include <algorithm>

namespace jstd {
    using namespace texturing;

    image_packer::image_packer() : 
    m_allocator(nullptr), 
    m_node_allocator(nullptr), 
    m_root(), 
    m_images(),
    m_width(0),
    m_height(0) {

    }

    image_packer::image_packer(const image* img_array, int32_t count_images, int32_t w, int32_t h, tca::base_allocator* allocator, tca::base_allocator* node_allocator) :
    m_allocator(allocator), 
    m_node_allocator(node_allocator), 
    m_root(), 
    m_images(img_array, count_images),
    m_width(w),
    m_height(h) {
        
    }

    image_packer::image_packer(image_packer&& packer) : 
    m_allocator(packer.m_allocator),
    m_node_allocator(packer.m_node_allocator),
    m_root(std::move(packer.m_root)),
    m_images(std::move(packer.m_images)),
    m_width(packer.m_width),
    m_height(packer.m_height) {
        packer.m_allocator      = nullptr;
        packer.m_node_allocator = nullptr;
        packer.m_width          = 0;
        packer.m_height         = 0;
    }
    
    image_packer& image_packer::operator=(image_packer&& packer) {
        if (&packer != this) {
            m_allocator         = packer.m_allocator;
            m_node_allocator    = packer.m_node_allocator;
            m_root              = std::move(packer.m_root);
            m_images            = std::move(packer.m_images);
            m_width             = packer.m_width;
            m_height            = packer.m_height;

            packer.m_allocator      = nullptr;
            packer.m_node_allocator = nullptr;
            packer.m_width          = 0;
            packer.m_height         = 0;
        }
        return *this;
    }

    image_packer::~image_packer() {

    }

    int64_t get_max_mem_size_for_max_image(array<const image>& images) {
        int64_t mem_size    = 0;
        for (int64_t i = 0; i < images.length; ++i) {
            const image& img = images[i];
            mem_size = std::max<int64_t>(mem_size, img.get_width() * img.get_height() * img.get_channels());
        }
        return mem_size;
    }

    void image_packer::create_tree() {
#ifndef NDEBUG
        if (m_allocator == nullptr) 
            throw_except<illegal_state_exception>("m_allocator must be != null");
        if (m_node_allocator == nullptr) 
            throw_except<illegal_state_exception>("m_node_allocator must be != null");
#endif//NDEBUG
        if (m_root.is_null())
            m_root  = unique_ptr<node>(m_node_allocator, node(m_width, m_height, m_node_allocator));
        BEGIN: 
        for (int64_t i = 0; i < m_images.length; ++i) {
            const image& img = m_images[i];
            node* n = m_root->put_image(img.get_width(), img.get_height(), i);
            if (n == nullptr) {
                /**
                 * Если не получилось вставить изображение, начинаем заного, 
                 * но увеличиваем конечное изображение в два раза.
                 */
                m_width  <<= 1;
                m_height <<= 1;
                *m_root = node(m_width, m_height, m_node_allocator);
                goto BEGIN;
            }
        }
    }

    image image_packer::pack(int32_t scale_factor, int32_t out_image_channels) {
#ifndef NDEBUG
        if (scale_factor <= 0)
            throw_except<illegal_argument_exception>("scale_factor %li is illegal", (long int) scale_factor);
        if (out_image_channels <= 0 || out_image_channels > 4)
            throw_except<illegal_argument_exception>("out_image_channels %li is illegal", (long int) scale_factor);
        if (m_allocator == nullptr) 
            throw_except<illegal_state_exception>("m_allocator must be != null");
        if (m_node_allocator == nullptr) 
            throw_except<illegal_state_exception>("m_node_allocator must be != null");
        if (m_root.is_null())
            create_tree();
#endif//NDEBUG
        
        image result(m_root->get_rect().w / scale_factor, m_root->get_rect().h / scale_factor, out_image_channels, m_allocator);

        /**
         * Размер временного буфера, куда будет сохранено масштабированное изображение.
         * Буфер делится на rescale, так как если изначальное изображение весит килобайт, то масштабированный на 2, размер будет в два раза меньше и так далее.
         */
        int64_t buffer_size_for_resized_image = get_max_mem_size_for_max_image(m_images) / scale_factor;

        /**
         * Линейный аллокатор, который нужен, чтобы выделять память под хранения временного масштабированного изображения
         */
        tca::linear_allocator allocator_for_rescaled_image; 
        
        //Если делитель больше 1, то масштабирование изображения требуется, а значит требуется и временный буфер.
        if (scale_factor > 1)
            allocator_for_rescaled_image = tca::linear_allocator(buffer_size_for_resized_image, m_allocator);

        struct callback_image {
            /**
             * Массив оригинальных изображений.
             */
            array<const image>*     m_array_of_image;
            
            /**
             * Указатель на результирующее изображение (атлас).
             * В который будет записано изображение.
             */
            image*                  m_atlas;
            
            /**
             * Делитель для уменьшения изображения.
             * Для корректных результатов должен быть кратен степени двойки. 2 4 8 16 32....1024
             */
            int32_t                 m_rescale;
            
            /**
             * Аллокатор для выделения памяти под уменьшенное изображение.
             * Поскольку void operator() (node*) вызывается каждый раз для каждого изображения, то внутри него предвыделить буфер невозможно.
             * 
             */
            tca::linear_allocator*  m_allocator_for_tmp_resized_image;

            //Я даже описывать не хочу, какого чёрта тут происходит. Stupid Fuck.
            void operator()(const node* n) {
                int32_t image_index = n->get_id();

                const image& img_original = (*m_array_of_image)[image_index];
                int32_t w_image = img_original.get_width();
                int32_t h_image = img_original.get_height();

                image rescaled_image; 
                if (m_rescale > 1 && m_allocator_for_tmp_resized_image != nullptr)
                    rescaled_image = img_original.resize(w_image / m_rescale, h_image / m_rescale, m_allocator_for_tmp_resized_image);

                const image& img = (m_rescale > 1) ? rescaled_image : img_original;

                /**
                 * Нода имеет координату, ширину и величину изображения.
                 * Причём ширина и высота всегда идентична изображению!
                 */
                const int32_t x = n->get_rect().x     / m_rescale;
                const int32_t y = n->get_rect().y     / m_rescale;
                const int32_t w = n->get_rect().w     / m_rescale;
                const int32_t h = n->get_rect().h     / m_rescale;
                
                if (m_atlas->get_channels() == 4) {
                    for (int32_t yo = 0; yo < h; ++yo) {
                        for (int32_t xo = 0; xo < w; ++xo) {
                            
                            const int32_t xx = xo + x;
                            const int32_t yy = yo + y;
                            
                            if (img.get_channels() == 4) {
                                m_atlas->get_rgba(xx, yy) = img.get_rgba(xo, yo);   
                            } 

                            else if (img.get_channels() == 3) {
                                const image::rgb& col = img.get_rgb(xo, yo);
                                m_atlas->get_rgba(xx, yy) = image::rgba(col.r, col.g, col.b, 0xff);
                            }
                            
                            else if (img.get_channels() == 1) {
                                image::byte col = img.get_gray(xo, yo).brightness;
                                m_atlas->get_rgba(xx, yy) = image::rgba(col, col, col, 0xff);
                            }

                            else {
                                throw_except<illegal_state_exception>("");
                            }  
                        }
                    }
                } 

                else if (m_atlas->get_channels() == 3) {
                    for (int32_t yo = 0; yo < h; ++yo) {
                        for (int32_t xo = 0; xo < w; ++xo) {
                            
                            const int32_t xx = xo + x;
                            const int32_t yy = yo + y;
                            
                            if (img.get_channels() == 4) {
                                const image::rgba& col = img.get_rgba(xo, yo);
                                m_atlas->get_rgb(xx, yy) = image::rgb(col.r, col.g, col.b);   
                            } 

                            else if (img.get_channels() == 3) {
                                m_atlas->get_rgb(xx, yy) = img.get_rgb(xo, yo);
                            }
                            
                            else if (img.get_channels() == 1) {
                                image::byte col     = img.get_gray(xo, yo).brightness;
                                m_atlas->get_rgb(xx, yy) = image::rgb(col, col, col);
                            }

                            else {
                                throw_except<illegal_state_exception>("");
                            }  
                        }
                    }
                } 
                
                else if (m_atlas->get_channels() == 1) {
                    
                    const float r_gamma = 0.30f;
                    const float g_gamma = 0.59f;
                    const float b_gamma = 0.11f;
                    image::byte gray    = 0;                            

                    for (int32_t yo = 0; yo < h; ++yo) {
                        for (int32_t xo = 0; xo < w; ++xo) {
                            
                            const int32_t xx = xo + x;
                            const int32_t yy = yo + y;
                            
                            
                            if (img.get_channels() == 4) {
                                const image::rgba& rgba = img.get_rgba(xo, yo);
                                gray = (image::byte) (rgba.r * r_gamma + rgba.g * g_gamma + rgba.b * b_gamma) / 3;
                            } 
                            
                            else if (img.get_channels() == 3) {
                                const image::rgb& rgb = img.get_rgb(xo, yo);
                                gray = (image::byte) (rgb.r * r_gamma + rgb.g * g_gamma + rgb.b * b_gamma) / 3;
                            }
                            
                            else if (img.get_channels() == 1) {
                                gray = img.get_gray(xo, yo).brightness;
                            }
                            
                            else {
                                throw_except<illegal_state_exception>("");
                            }  
                            m_atlas->get_gray(xx, yy).brightness = gray;   
                        }
                    }
                }

                else {
                    throw_except<illegal_state_exception>();
                }

                if (m_allocator_for_tmp_resized_image != nullptr)
                    m_allocator_for_tmp_resized_image->reset();
            }
        };

        callback_image callback;
        callback.m_array_of_image   = &m_images;
        callback.m_atlas            = &result;
        callback.m_rescale          = scale_factor;
        callback.m_allocator_for_tmp_resized_image = scale_factor > 1 ? &allocator_for_rescaled_image : nullptr;

        m_root->depth_search(callback);

        return image(std::move(result));
    }

    array<image_packer::uv> image_packer::get_uv() const {
        array<image_packer::uv> uvs(m_images.length, m_allocator);
        
        struct node_visitor {
            array<image_packer::uv>* m_array;
            void operator() (const node* n) {
                const rect& pos = n->get_rect();
                const int32_t u0 = pos.x;
                const int32_t v0 = pos.y;
                const int32_t u1 = pos.x + pos.w;
                const int32_t v1 = pos.y + pos.h;
                image_packer::uv texcoord;
                texcoord.u0 = u0;
                texcoord.v0 = v0;
                texcoord.u1 = u1;
                texcoord.v1 = v1;
                (*m_array)[n->get_id()] = texcoord;
            }
        };

        node_visitor visitor;
        visitor.m_array = &uvs;
        
        m_root->depth_search(visitor);

        return array<image_packer::uv>(std::move(uvs));
    }
}