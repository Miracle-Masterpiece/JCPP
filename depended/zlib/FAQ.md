# How to compile?

<details>
<summary> 📚 zlib </summary>

### Windows

`gcc -O2 -o libzlib.dll -fPIC -shared src/*.c`

### Linux

`gcc -O2 -o libzlib.so -shared -fPIC src/*.c`

The compilation was also tested on clang.

</details>