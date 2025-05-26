# What it this project?
**The project is a kind of standard library in the spirit of the Java programming language, with additions such as polymorphic allocators for flexible work with memory, objects, arrays, etc.**

# What frameworks does this library implement?

<details>
<summary> 📚 Collection Framework (Click to expand) </summary>

**using namespace jstd;**
- `array`
- `array_list` (Array base list)
- `linked_list` (Doubly linked list)
- `hash_map`
- `hash_set`
</details>


<details>
<summary> 💾 IO Framework (Click to expand) </summary>

**using namespace jstd;**

<details>
<summary> Input stream </summary>
     
- `istream` (base class for input stream)
- `ifstream` (input file stream)
- `idstream` (input data stream)
- `ibstream` (input buffered stream)
- `iastream` (input array stream)
- `inflstream` (inflater stream)

</details>
    
<details>
<summary> Output stream </summary>
     
- `ostream` (base class for output stream)
- `ofstream` (output file stream)
- `odstream` (output data stream)
- `obstream` (output buffered stream)
- `oastream` (output array stream)
- `deflstream` (deflater stream)

</details>

- `file` (To represent paths, files, and directories)
- `properties` (For storing properties)
- `bytebuffer` (For working with raw bytes. Reading and writing primitive types)

</details>

<details>
<summary> 🌐 NetFramework (Click to expand) </summary>

**using namespace jstd;**

 _For sockets to work, the network subsystem must be initialized. To initialize the functions, you need to call the "init_init()" function from the "include/cpp/lang/net/inet.hpp" header file. Also, to disable the network subsystem, call the close_inet() function"_

- `socket` (To work with a TCP connection) 
- `inetaddr` (For represent inet address IPv4 & IPv6) 
- `socket_option` (To configure TCP connection options)

</details>

<details>
<summary> 🎲 Objects hash & equals </summary>

**using namespace jstd;**

In file **#include <cpp/lang/utils/hash.hpp>**

Specializations of these structures are used to define an implicit pattern in places where comparison and hash code functors are used.

- `struct hash_for` (A functor object for defining a standard hash code)
- `struct equal_to` (A functor object for defining a standard comparison)

</details>

# How Build?

<details> 
<summary>Depended</summary>

#### Needed depended
##### Total: <a href="https://github.com/madler/zlib.git"> `zlib` </a>
##### Windows: `WinSock2`

#### Optional depended
The entire project was originally based on `-std=c++11`, but eventually it was decided to add support for `-std=c++23` to use stack traces in exceptions. So, if you are using c++23 and the GCC compiler, then you need to linkage `stdc++exp`. It was not tested on other compilers.

</details>


<details>
<summary> How compile? </summary>

_Yes, compiling this library manually is a dark ritual. Good luck._ :D


### Windows:

##### C++11
`g++ -std=c++11 -O0 -o jcpp.exe -I"include" src/base_allocator.cpp src/base_socket.cpp src/bsd_socket.cpp src/bsd_socket_class.cpp src/bytebuffer.cpp src/cstr.cpp src/date.cpp src/deflater.cpp src/deflater_inflater_funcs.cpp src/deflstream.cpp src/exceptions.cpp src/file.cpp src/filesystem.cpp src/iastream.cpp src/ibstream.cpp src/idstream.cpp src/ifstream.cpp src/image.cpp src/image_packer.cpp src/image_tree.cpp src/imageio.cpp src/inet.cpp src/inetaddr.cpp src/inflater.cpp src/inflstream.cpp src/internal_funcs.cpp src/iostream.cpp src/linear_allocator.cpp src/linear_compact_allocator.cpp src/malloc_free_allocator.cpp src/mutex.cpp src/null_allocator.cpp src/numbers.cpp src/oastream.cpp src/objects.cpp src/obstream.cpp src/odstream.cpp src/ofstream.cpp src/os_allocator.cpp src/pool_allocator.cpp src/properties.cpp src/shared_allocator.cpp src/shared_control_block.cpp src/shared_ptr.cpp src/stacktrace.cpp src/string.cpp src/subsystem.cpp src/system.cpp src/thread.cpp src/utils.cpp src/wav_data.cpp -lws2_32 -lzlib`

##### C++23
`g++ -std=c++23 -O0 -o jcpp.exe -I"include" src/base_allocator.cpp src/base_socket.cpp src/bsd_socket.cpp src/bsd_socket_class.cpp src/bytebuffer.cpp src/cstr.cpp src/date.cpp src/deflater.cpp src/deflater_inflater_funcs.cpp src/deflstream.cpp src/exceptions.cpp src/file.cpp src/filesystem.cpp src/iastream.cpp src/ibstream.cpp src/idstream.cpp src/ifstream.cpp src/image.cpp src/image_packer.cpp src/image_tree.cpp src/imageio.cpp src/inet.cpp src/inetaddr.cpp src/inflater.cpp src/inflstream.cpp src/internal_funcs.cpp src/iostream.cpp src/linear_allocator.cpp src/linear_compact_allocator.cpp src/malloc_free_allocator.cpp src/mutex.cpp src/null_allocator.cpp src/numbers.cpp src/oastream.cpp src/objects.cpp src/obstream.cpp src/odstream.cpp src/ofstream.cpp src/os_allocator.cpp src/pool_allocator.cpp src/properties.cpp src/shared_allocator.cpp src/shared_control_block.cpp src/shared_ptr.cpp src/stacktrace.cpp src/string.cpp src/subsystem.cpp src/system.cpp src/thread.cpp src/utils.cpp src/wav_data.cpp -lws2_32 -lzlib -lstdc++exp`

### Linux

</details>


