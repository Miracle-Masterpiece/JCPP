#ifndef JSTD_CPP_LANG_UTILS_IMAGES_IMAGEIO_H
#define JSTD_CPP_LANG_UTILS_IMAGES_IMAGEIO_H

#include <cpp/lang/utils/images/image.hpp>
#include <cpp/lang/io/istream.hpp>
#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/io/file.hpp>

namespace jstd {
    namespace imageio {
        /**
         * Загружает изображение из входящего потока.
         * 
         * @param in
         *      Входящий поток ввода.
         * 
         * @param allocator
         *      Распределитель памяти для выделения памяти под временный буфер и конечного изображения.
         * 
         * @throws out_of_memory_error
         *      Если памяти для временного буфера не хватило.
         *      Если прямяти для конечного изображения не хватило.
         * 
         * @throws io_exception
         *      Если произошла ошибка ввода/вывода
         * 
         * @throws illegal_state_exception
         *      Если произошла ошибка во внутренней библитеки.
         */
        image load_image(istream* in, tca::base_allocator* allocator);

        /**
         * Тот же самый, метод, что и {
         * @code image load_image(istream*, tca::base_allocator*)
         * }
         * 
         * Только вместо потока, файл из которого считать изображение.
         * 
         * @throws file_not_found_exception
         *      Если файла не существует.
         * 
         * Выбрасываемые исключения, такие-же, как и у функции {@code load_image(istream*, tca::base_allocator*)}.
         */
        image load_image(const file& file , tca::base_allocator* allocator);

        /**
         * Сохраняет изображение в поток вывода.
         * 
         * @param out
         *      Поток вывода поток ввода.
         * 
         * @param img
         *      Указатель на изображение, которое нужно сохранить.
         * 
         * @param ext
         *      Указатель на строку, отвечаюшую за тип сохранённого изображения.
         *      Например:
         *      "png", "jpeg" or "jpg", "bmp", "tga"
         * 
         * @throws io_exception
         *      Если произошла ошибка ввода/вывода
         * 
         * @throws illegal_state_exception
         *      Если произошла ошибка во внутренней библитеки.
         */
        void write_image(ostream* out, const image* img, const char* ext);
        
        /**
         * Тот же самый, метод, что и {
         * @code void save_image(ostream*, tca::base_allocator*)
         * }
         * 
         * Только вместо потока, файл в который сохранить изображение.
         * Выбрасываемые исключения, такие-же, как и у функции {@code void write_image(ostream*, const image*, const char*)}.
         */
        void write_image(const file& file, const image* img, const char* ext);

        

    }//namespace imageio
}//namespace jstd 
#endif//JSTD_CPP_LANG_UTILS_IMAGES_IMAGEIO_H