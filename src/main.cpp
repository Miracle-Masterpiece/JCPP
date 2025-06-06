#include <iostream>
#include <allocators/linear_allocator.hpp>
#include <allocators/shared_allocator.hpp>
#include <allocators/os_allocator.hpp>
#include <cpp/lang/ustring.hpp>
#include <cpp/lang/utils/array_list.hpp>
#include <cpp/lang/io/bytebuffer.hpp>
#include <cpp/lang/io/utility.hpp>
#include <cpp/lang/utils/shared_ptr.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/utils/hash_map.hpp>
#include <cpp/lang/net/socket.hpp>
#include <cpp/lang/net/inet.hpp>
#include <cpp/lang/utils/images/imageio.hpp>
#include <cpp/lang/utils/images/image_packer.hpp>
#include <cpp/lang/utils/images/image.hpp>
#include <allocators/inline_linear_allocator.hpp>
#include <allocators/null_allocator.hpp>
#include <cpp/lang/concurrency/thread.hpp>
#include <allocators/linear_compact_allocator.hpp>
#include <allocators/pool_allocator.hpp>
#include <cpp/lang/utils/shared_ptr.hpp>
#include <cpp/lang/utils/limits.hpp>
#include <cpp/lang/utils/random.hpp>
#include <cpp/lang/utils/smooth_noise.hpp>
#include <cpp/lang/math/math.hpp>
#include <cpp/lang/utils/linked_list.hpp>
#include <allocators/arena_free_list_allocator.hpp>

namespace cstd = jstd;



using namespace jstd;


tca::os_allocator os_alloc;
//tca::shared_allocator m_alloc(&os_alloc, 1 << 18, tca::shared_allocator::BEST_FIT);

cstd::array<cstd::image> load_all_image(const char* path, tca::base_allocator* allocator = tca::get_scoped_or_default()) {
    using namespace jstd;
    
    struct png_filter : file_filter {
        bool apply(const char* name, int len) const override {
            tca::inline_linear_allocator<io::constants::MAX_LENGTH_PATH> str_path_allocator;
            tca::scope_allocator scope = &str_path_allocator;
            string file_name = name;
            if (file_name.ends_with(".png"))
                return true;
            return false;
        }
    };
    
    tca::scope_allocator scope_alloc = allocator;
    png_filter filter;
    
    array<file> images_files = file(path).list_files(&filter);
    
    array<image> imgs(images_files.length);

    int idx = 0;
    for (const file& f : images_files)
        imgs[idx++] = imageio::load_image(f.str_path());

    return array<image>(std::move(imgs));
}

int main() {
    // using namespace jstd;
    // init_inet();

    // try {
    //     tca::arena_free_list_allocator m_alloc(1024 * 1024 * 32, &os_alloc);
    //     tca::scope_allocator scope = &m_alloc;
    //     array<image> pngs = load_all_image("./images/");
    //     image_packer packer(pngs.data(), pngs.length, 128, 128, tca::get_scoped_or_default());
    //     image packed = packer.pack(2);
    //     m_alloc.print();
    //     std::cout << "\n\n\n" << std::endl;
    //     imageio::write_image(file("./atlas.png"), &packed, "png");
    // } catch (const throwable& t) {
    //     std::cout << t.cause() << std::endl;
    //     t.print_stack_trace();
    // }
    // close_inet();

    try {
        // const int N_SIZE = 4096;
        
        // random rnd;
        // smooth_noise noise(36, math::quintic);
        // image noise_image(N_SIZE, N_SIZE, 1);
        
        // for (int y = 0; y < N_SIZE; ++y) {
        //     for (int x = 0; x < N_SIZE; ++x) {
        //         image::byte c = (image::byte) (noise.get(x, y, 256, rnd.next<int>(5) + 1) * 255);
        //         //std::cout << (int) c << std::endl;
        //         //noise_image.get_rgb(x, y) = {.r = c, .g = c, .b = c};
        //         noise_image.get_gray(x, y).gray = c;
        //     }   
        // }

        // imageio::write_image(file("./map.png"), &noise_image, "png");

        struct _int {
            int value;
        public:
            _int(int value = 0) : value(value) {
                std::cout << "_int(int)" << "\n";
            }
            _int(const _int& i) : value(i.value) {
                std::cout << "_int(const int&)" << "\n";
            }
            _int(_int&& i) : value(i.value) {
                std::cout << "_int(int&&)" << "\n";
            }
            _int& operator= (const _int& i) {
                value = i.value;
                std::cout << "_int& operator= (const int&)" << "\n";
                return *this;
            }
            _int& operator= (_int&& i) {
                value = i.value;
                std::cout << "_int& operator= (int&&)" << "\n";
                return *this;
            }
            ~_int() {
                std::cout << "~_int()" << "\n";
            }
            operator int() {
                return value;
            }
        };

        tca::scope_allocator malloc_scope = tca::get_default_allocator();
        tca::arena_free_list_allocator allocator(1024); 
        tca::scope_allocator scoped = &allocator;

        string s = "hello";
        std::cout << "capacity: " << s.capacity() << std::endl;
        std::cout << "length: " << s.length() << std::endl;
        s.clear();
        s.trim_to_size();
        std::cout << "capacity: " << s.capacity() << std::endl;
        std::cout << "length: " << s.length() << std::endl;
        
        // shared_ptr<_int> ptr = make_shared<_int>(546);
        // weak_ptr<_int> wptr = ptr;
        // shared_ptr<_int> _ptr = wptr.lock();        

        // std::cout << *_ptr << std::endl;
        // std::cout << obj_to_cstr_buf(_ptr) << std::endl;

        // // linked_list<_int> list = {1, 2, 3, 4, 5};
        // // list.remove_at(0);

        // // for (_int& entry : list) {
        // //     std::cout << entry << "\n";    
        // // }

        allocator.print();

    } catch(const throwable& e) {
        std::cout << e.cause() << std::endl;
        e.print_stack_trace();
    }
    

    


    return 0;
}