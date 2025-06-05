#include <cpp/lang/utils/images/image.hpp>
#include <cpp/lang/exceptions.hpp>
#include <utility>
#include <cpp/lang/utils/unique_ptr.hpp>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <cpp/lang/utils/images/stb_image_resize.h>

namespace jstd {

    image::image() :
    m_allocator(nullptr),
    m_pixels(nullptr),
    m_width(0),
    m_height(0),
    m_channels(0) {

    }
    
    image::image(int32_t width, int32_t height, int8_t channels, tca::base_allocator* allocator) : image() {
        byte* pixels = reinterpret_cast<byte*>(allocator->allocate_align(width * height * channels, alignof(byte)));
        if (pixels == nullptr)
            throw_except<out_of_memory_error>("Out of memory!");
        std::memset(pixels, 0, width * height * channels);
        m_allocator = allocator;
        m_pixels    = pixels;
        m_width     = width;
        m_height    = height;
        m_channels  = channels;
    }
    
    void image::cleanup() {
        if (m_allocator != nullptr && m_pixels != nullptr) {
            m_allocator->deallocate(m_pixels, m_width * m_height * m_channels);
            m_pixels    = nullptr;
            m_allocator = nullptr;
        }
    }

    image::~image() {
        cleanup();
    }

    image::image(const image& img) : image() {
        image tmp = img.clone();
        *this = std::move(tmp);
    }

    image& image::operator= (const image& img) {
        if (&img != this) {
            image tmp = img.clone();
            *this = std::move(tmp);
        }
        return *this;
    }

    image::image(image&& img) : 
    m_allocator(img.m_allocator), 
    m_pixels(img.m_pixels), 
    m_width(img.m_width), 
    m_height(img.m_height), 
    m_channels(img.m_channels) {
        img.m_allocator = nullptr;
        img.m_pixels    = nullptr;
        img.m_width     = 0;
        img.m_height    = 0;
        img.m_channels  = 0;
    }

    image& image::operator= (image&& img) {
        if (&img != this) {
            cleanup();
            m_allocator = img.m_allocator;
            m_pixels    = img.m_pixels;
            m_width     = img.m_width;
            m_height    = img.m_height;
            m_channels  = img.m_channels;
            
            img.m_allocator = nullptr;
            img.m_pixels    = nullptr;
            img.m_width     = 0;
            img.m_height    = 0;
            img.m_channels  = 0;
        }
        return *this;
    }
    
    int32_t image::get_width() const {
        return m_width;
    }

    int32_t image::get_height() const {
        return m_height;
    }

    image::byte* image::pixels() {
        return m_pixels;
    }
    
    const image::byte* image::pixels() const {
        return m_pixels;
    }

    int8_t image::get_channels() const {
        return m_channels;
    }

    image::rgb& image::get_rgb(int32_t x, int32_t y) {
        JSTD_DEBUG_CODE(
            if (m_channels != 3)
                throw_except<illegal_state_exception>("image is not rgb");
            check_index(x, m_width);
            check_index(y, m_height);
        );
        int idx = x + y * m_width;
        return m_rgb[idx];
    }

    const image::rgb& image::get_rgb(int32_t x, int32_t y) const {
        JSTD_DEBUG_CODE(
            if (m_channels != 3)
                throw_except<illegal_state_exception>("image is not rgb");
            check_index(x, m_width);
            check_index(y, m_height);
        );
        int idx = x + y * m_width;
        return m_rgb[idx];
    }

    image::rgba& image::get_rgba(int32_t x, int32_t y) {
        JSTD_DEBUG_CODE(
            if (m_channels != 4)
                throw_except<illegal_state_exception>("image is not rgba");
            check_index(x, m_width);
            check_index(y, m_height);
        );
        int idx = x + y * m_width;
        return m_rgba[idx];
    }

    const image::rgba& image::get_rgba(int32_t x, int32_t y) const {
        JSTD_DEBUG_CODE(
            if (m_channels != 4)
                throw_except<illegal_state_exception>("image is not rgba");
            check_index(x, m_width);
            check_index(y, m_height);
        );
        int idx = x + y * m_width;
        return m_rgba[idx];
    }

    image::gray& image::get_gray(int32_t x, int32_t y) {
        JSTD_DEBUG_CODE(
            if (m_channels != 1)
                throw_except<illegal_state_exception>("image is not gray");
            check_index(x, m_width);
            check_index(y, m_height);
        );
        int idx = x + y * m_width;
        return m_gray[idx];
    }

    const image::gray& image::get_gray(int32_t x, int32_t y) const {
        JSTD_DEBUG_CODE(
            if (m_channels != 1)
                throw_except<illegal_state_exception>("image is not gray");
            check_index(x, m_width);
            check_index(y, m_height);
        );
        int idx = x + y * m_width;
        return m_gray[idx];
    }

    int image::rgba::to_string(char buf[], int bufsize) const {
        return snprintf(buf, bufsize, "[R: %i, G: %i, B: %i, A: %i]", (int) r, (int) g, (int) b, (int) a);
    }

    int image::rgb::to_string(char buf[], int bufsize) const {
        return snprintf(buf, bufsize, "[R: %i, G: %i, B: %i]", (int) r, (int) g, (int) b);
    }

    int image::gray::to_string(char buf[], int bufsize) const {
        return snprintf(buf, bufsize, "[GRAY: %i]", (int) gray);
    }

    image image::resize(int32_t neww, int32_t newh, tca::base_allocator* allocator) const {
        if (neww <= 0 || newh <= 0)
            throw_except<illegal_argument_exception>("Invalid width or height");
        if (allocator == nullptr) {
            if (m_allocator == nullptr)
                return image();
            allocator = m_allocator;
        }
        image resized_image(neww, newh, m_channels, allocator);
        int error = stbir_resize_uint8(reinterpret_cast<const unsigned char*>(m_pixels), m_width, m_height, 0, resized_image.pixels(), neww, newh, 0, m_channels);
        if (error == 0)
            throw_except<illegal_state_exception>("stbi_resize error: %i", error);
        return image(std::move(resized_image));
    }

    image image::clone(tca::base_allocator* allocator) const {
        if (allocator == nullptr) {
            if (m_allocator == nullptr)
                return image();
            allocator = m_allocator;
        }
        image img(m_width, m_height, m_channels, allocator);
        std::memcpy(img.m_pixels, m_pixels, sizeof(byte) * (m_width * m_height * m_channels));
        return image(std::move(img));
    }

    int image::to_string(char buf[], int bufsize) const {
        return snprintf(buf, bufsize, "[w: %i, h: %i, channels: %i]", (int) m_width, (int) m_height, (int) m_channels);
    }

}

