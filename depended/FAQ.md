# How to compile?

<details>
<summary> 📚 zlib </summary>

### Windows

`gcc -O2 -o libzlib.dll -shared zlib/*.c`

### Linux

`gcc -O2 -o libzlib.so -shared -fPIC zlib/*.c`

The compilation was also tested on clang.

</details>