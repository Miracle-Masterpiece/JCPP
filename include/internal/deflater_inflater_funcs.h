namespace jstd {
namespace zip {
namespace internal {
    /**
     * Возвращает строковое представление ошибки в библиотеки сжатия.
     * 
     * @warning
     *      Строка является Implemented Defined, описание ошибки может меняться от версии к версии.
     */
    const char* zlib_errorcode_to_string(int err);
}//namespace internal
}//namespace zip
}//namespace jstd