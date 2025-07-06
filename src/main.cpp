#include <iostream>
#include <cpp/lang/io/file_channel.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/utils.hpp>
#include <cpp/lang/io/ofstream.hpp>
#include <cpp/lang/io/ifstream.hpp>
#include <cpp/lang/io/odstream.hpp>
#include <cpp/lang/io/idstream.hpp>

// void write_with_order_int(void* ptr, int v, jstd::byte_order out_order) {
//     unsigned char* const out    = reinterpret_cast<unsigned char*>(ptr);
//     unsigned char* const in     = reinterpret_cast<unsigned char*>(&v);
//     const std::size_t T_SIZE    = sizeof(int);
//     //if (out_order == jstd::system::native_byte_order()) {
//     //    std::memcpy(out, in, T_SIZE);
//     //}
//     //else {
//         unsigned i = 0;
//         while (i < T_SIZE) {
//             out[i] = in[T_SIZE - i - 1];
//             ++i;
//         }
//     //}
// }

void write_with_order_int(void* ptr, uint32_t v, jstd::byte_order out_order) {
    if (out_order != jstd::system::native_byte_order())
        v = jstd::utils::bswap<uint32_t>(v);
    std::memcpy(ptr, &v, sizeof(uint32_t));
}


// void copy_swap_memory_test(void* dst, const void* src, std::size_t n) {
//     std::size_t i = 0;
//     for (std::size_t i = 0; i < n; ++i)
//         ((T*) dst)[i] = jstd::utils::bswap<T>(((T*) src)[i++]);
// }

uint32_t read_with_order_int(const void* ptr, jstd::byte_order out_order) {
    uint32_t v;
    std::memcpy(&v, ptr, sizeof(uint32_t));
    if (out_order != jstd::system::native_byte_order())
        v = jstd::utils::bswap<uint32_t>(v);
    return v;
}

int main(int argc, const char** args) {
    const int N_SIZE = 1024;
    try {
    
        jstd::file_channel file("./Привет!.txt", jstd::open_option::CREATE | jstd::open_option::READ);
        file.truncate(16);
        jstd::mapped_byte_buffer buf1 = file.map(jstd::fmap_mode::READ_ONLY, 0, 16);
        //jstd::mapped_byte_buffer buf2 = file.map(jstd::fmap_mode::READ_ONLY, 16, 16);

        // for (int i = 0; i < 16; ++i) {
        //     buf1.put<char>('a' + i);
        // }

        for (int i = 0; i < 16; ++i) {
            std::cout << buf1.get<char>(i) << std::endl;
        }

        // for (int i = 0; i < 16; ++i) {
        //     buf2.put<char>('A' + i);
        // }

        buf1.force();
    
    } catch (jstd::sequrity_exception& t) {
        std::cout << "seq\n";
        std::cout << t.cause() << "\n";
        t.print_stack_trace();
    } catch (jstd::throwable& t) {
        std::cout << t.cause() << "\n";
        t.print_stack_trace();
    }

    std::cout << "exit\n"; 

    return 0;
}