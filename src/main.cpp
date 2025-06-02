#include <iostream>
#include <allocators/linear_allocator.hpp>
#include <allocators/shared_allocator.hpp>
#include <allocators/os_allocator.hpp>
#include <cpp/lang/ustring.hpp>
#include <cpp/lang/utils/array_list.hpp>
#include <cpp/lang/io/bytebuffer.hpp>
#include <cpp/lang/utils/shared_ptr.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/utils/hash_map.hpp>


int main() {
    using namespace jstd;

    std::max_align_t buffer[1024];
    tca::linear_allocator linear_alloc(buffer, sizeof(buffer));
    tca::os_allocator __os;
    tca::shared_allocator m_alloc(&__os);

    try {
        tca::scope_allocator scope = &m_alloc;
        hash_map<int, int> map;
        for (int i = 0; i < 100; ++i) {
            map.put(i, i);
        }
        for (const map_node<int, int>& entry : map) {
            int k = entry.get_key();
            int v = entry.get_value();
            std::cout << k << " = " << v << std::endl;
        }
        for (int i = 0; i < 100; ++i) {
            map.remove(i);
        }

    } catch (const throwable& t) {
        t.print_stack_trace();
    }

    m_alloc.merge_free_blocks();
    m_alloc.print_log();
    linear_alloc.print();
}