#include <cpp/lang/utils/images/imageio.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/unique_ptr.hpp>
#include <allocators/malloc_free_allocator.hpp>
#include <cpp/lang/io/ifstream.hpp>
#include <cpp/lang/io/ofstream.hpp>
#include <cpp/lang/io/iostream.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG

/**
 * Определение макросов, чтобы stbimage использовал фукнции распреления памяти из jstd
 */
#define STBI_MALLOC(size) jstd_malloc(size)
#define STBI_REALLOC(pointer, new_size) jstd_realloc(pointer, new_size)
#define STBI_FREE(pointer) jstd_free(pointer)

#define STBIW_FAILURE_USERMSG
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define STBIW_MALLOC(size) jstd_malloc(size)
#define STBIW_REALLOC(pointer, new_size) jstd_realloc(pointer, new_size)
#define STBIW_FREE(pointer) jstd_free(pointer)

static void* jstd_malloc(std::size_t sz);
static void* jstd_realloc(void* p, std::size_t sz);
static void jstd_free(void* ptr);

#include <cpp/lang/utils/images/stb_image.h>
#include <cpp/lang/utils/images/stb_image_write.h>

static void* jstd_malloc(std::size_t sz) {
    tca::malloc_free_allocator jstd_malloc_allocator;
    return jstd_malloc_allocator.allocate(sz);
}

static void* jstd_realloc(void* p, std::size_t sz) {
    tca::malloc_free_allocator jstd_malloc_allocator;
    return jstd_malloc_allocator.reallocate(p, sz);
}

static void jstd_free(void* ptr) {
    tca::malloc_free_allocator jstd_malloc_allocator;
    return jstd_malloc_allocator.deallocate(ptr, -1);
}

namespace jstd 
{

namespace imageio 
{

    image load_image(istream* in, tca::base_allocator* allocator) {        
        int64_t size = in->available();
        unique_ptr<stbi_uc[]> raw_image(allocator, size);
        
        in->read((char*) raw_image.raw_ptr(), size);
    
        int width;
        int heigth;
        int channels;
        
        /**
         * Простая обёртка над malloc/free
         */
        tca::malloc_free_allocator jstd_malloc_allocator;
        unique_ptr<stbi_uc> pixels(&jstd_malloc_allocator, stbi_load_from_memory(raw_image.raw_ptr(), size, &width, &heigth, &channels, 0));
        
        if (pixels.is_null())
            throw_except<illegal_state_exception>("%s", stbi_failure_reason());
        
        image img(width, heigth, channels, allocator);
        
        image::byte* img_raw_data = img.pixels();
        int32_t len = width * heigth * channels;
        
        for (int32_t i = 0; i < len; ++i)
            img_raw_data[i] = (image::byte) pixels[i];
        
        // Собственно, данная штука, конечно, нужна. 
        // Но поскольку выделитель памяти переопределён в stbimage, то я и так знаю, что unique_ptr правильно освободит память.
        //
        // stbi_image_free(pixels);
        return img;
    }

    image load_image(const file& file, tca::base_allocator* allocator) {
        ifstream in(file);
        image img;
        try {
            img = std::move(load_image(&in, allocator));
        } catch (...) {
            close_stream_and_suppress_except(&in);    
            throw;
        }
        in.close();
        return image(std::move(img));
    }
    
    static void save_contex(void* context, void* data, int size){
		ostream* out = reinterpret_cast<ostream*>(context);
        out->write(reinterpret_cast<const char*>(data), size);
	}

    void write_image(const file& file, const image* img, const char* ext) {
        ofstream out(file);
        try {
            write_image(&out, img, ext);
        } catch (...) {
            close_stream_and_suppress_except(&out);
            throw;
        }
        out.close();
    }

    void write_image(ostream* out, const image* img, const char* ext) {
        int width       = img->get_width();
        int height      = img->get_height();
        int channels    = img->get_channels();
        
        int error = 0;

        if (std::strcmp(ext, "png") == 0) {
            error = stbi_write_png_to_func(save_contex, out, width, height, channels, img->pixels(), 0);
        } 
        
        else if (std::strcmp(ext, "jpeg") == 0|| std::strcmp(ext, "jpg")) {
            error = stbi_write_jpg_to_func(save_contex, out, width, height, channels, img->pixels(), 100);
        } 

        else if (std::strcmp(ext, "tga") == 0) {
            error = stbi_write_tga_to_func(save_contex, out, width, height, channels, img->pixels());
        }
     
        else if (std::strcmp(ext, "bmp") == 0) {
            error = stbi_write_bmp_to_func(save_contex, out, width, height, channels, img->pixels());
        } 

        if (error == 0)
            throw_except<illegal_state_exception>("stbi_write error: %i", error);
    }

}

}