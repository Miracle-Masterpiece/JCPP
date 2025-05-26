# How to compile?

<details>
<summary> 📚 zlib </summary>

### Windows

`gcc -Wall -o zlib.dll -shared zlib/gzlib.c zlib/trees.c zlib/uncompr.c zlib/zutil.c zlib/adler32.c zlib/compress.c zlib/crc32.c zlib/deflate.c zlib/gzclose.c zlib/gzread.c zlib/gzwrite.c zlib/infback.c zlib/inffast.c zlib/inflate.c zlib/inftrees.c`

### Linux

`gcc -Wall -o zlib.so -shared -fPIC zlib/gzlib.c zlib/trees.c zlib/uncompr.c zlib/zutil.c zlib/adler32.c zlib/compress.c zlib/crc32.c zlib/deflate.c zlib/gzclose.c zlib/gzread.c zlib/gzwrite.c zlib/infback.c zlib/inffast.c zlib/inflate.c zlib/inftrees.c`

The compilation was also tested on clang.

</details>