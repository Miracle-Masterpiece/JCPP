#ifndef JSTD_LANG_UTILS_AUDIO_WAV_DATA_H
#define JSTD_LANG_UTILS_AUDIO_WAV_DATA_H

#include <allocators/base_allocator.hpp>
#include <cpp/lang/io/istream.hpp>
#include <cstdint>

namespace jstd
{

/**
 * Представляет декодированные заголовки и данные WAV-файла.
 * 
 * Этот класс предоставляет низкоуровневый доступ к структурам WAV-формата,
 * включая заголовочные поля RIFF, формат аудио, частоту дискретизации, 
 * количество каналов, битность, а также сами аудиоданные. 
 * 
 * Загрузка производится вручную из входного потока или по пути к файлу.
 * Все выделения памяти производятся через пользовательский аллокатор.
 */
class wav_data {
    tca::base_allocator* m_allocator;
    int32_t     chunkId;
    uint32_t    chunkSize;
    int32_t     format;
    int32_t     subchunk1Id;
    uint32_t    subchunk1Size;
    int16_t     audioFormat;
    int16_t     numChannels;
    int32_t     sampleRate;
    int32_t     byteRate;
    int16_t     blockAlign;
    int16_t     bitsPerSample;
    int32_t     subchunk2Id;
    uint32_t    subchunk2Size;
    char*       data;

    /**
     * Загружает WAV-данные из указанного входного потока.
     *
     * @param in
     *      Указатель на входной поток (например, файл или буфер), содержащий WAV-данные.
     *      Должен быть валидным и не равным nullptr.
     * 
     * @throws std::runtime_error
     *      Если формат WAV недопустим или при чтении произошла ошибка.
     */
    void load_from(/*!non null!*/istream* in);

    /**
     * Освобождает ресурсы, выделенные при загрузке WAV-данных.
     */
    void cleanup();

public:
    /**
     * Создаёт пустой объект wav_data.
     * 
     * После вызова необходимо вручную вызвать метод load_from()
     * для загрузки данных WAV из потока.
     */
    wav_data();

    /**
     * Загружает WAV-данные из файла.
     *
     * @param path
     *      Путь к .wav-файлу. Должен быть строкой в формате UTF-8.
     * 
     * @param allocator
     *      Пользовательский аллокатор памяти, который будет использоваться для хранения аудиоданных.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     * 
     * @throws eof_exception
     *      Если данные неожиданно закончились.
     * 
     * @throws invalid_data_format_exception
     *      Если формат WAV недопустим
     */
    wav_data(const char* path, tca::base_allocator* allocator);

    /**
     * Загружает WAV-данные из указанного входного потока.
     * 
     * @param in
     *      Указатель на поток ввода, содержащий WAV-данные.
     *      Должен быть валидным и не равным nullptr.
     * 
     * @param allocator
     *      Пользовательский аллокатор памяти, используемый для хранения аудиоданных.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     * 
     * @throws eof_exception
     *      Если данные неожиданно закончились.
     * 
     * @throws invalid_data_format_exception
     *      Если формат WAV недопустим
     */
    wav_data(istream* in, tca::base_allocator* allocator);

    /**
     * Освобождает все ресурсы, связанные с WAV-данными.
     */
    ~wav_data();

    /**
     * Минимально рекомендуемый размер буфера для метода {@link to_string()}.
     */
    static const int32_t TO_STRING_MIN_BUFFER_SIZE = 256 + 128;

    /**
     * Записывает текстовое представление WAV-данных в указанный буфер.
     * 
     * @param buf
     *      Буфер, в который будет записано строковое представление.
     * 
     * @param bufsize
     *      Размер буфера. Должен быть не меньше {@link TO_STRING_MIN_BUFFER_SIZE}.
     * 
     * @return
     *      Количество записанных символов, не включая завершающий нуль-символ.
     */
    int32_t to_string(char buf[], int32_t bufsize) const;

    /**
     * @return
     *      Указатель на необработанные PCM-данные WAV-файла.
     */
    const char* get_data() const;

    /**
     * @return
     *      Формат WAV-аудио.
     */
    int32_t get_format() const;

    /**
     * @return
     *      Частота дискретизации (в герцах).
     */
    int32_t get_sample_rate() const;

    /**
     * @return
     *      Количество байт, передаваемых в секунду.
     */
    int32_t get_byte_rate() const;

    /**
     * @return
     *      Длина аудиоданных в байтах.
     */
    uint32_t get_length() const;

    /**
     * @return
     *      Количество звуковых каналов (1 — моно, 2 — стерео и т.д.).
     */
    int16_t get_num_channels() const;

    /**
     * @return
     *      Количество бит на один сэмпл.
     */
    int16_t get_bits_per_sample() const;
};


}

#endif//JSTD_LANG_UTILS_AUDIO_WAV_DATA_H