#ifndef JSTD_LANG_UTILS_AUDIO_WAV_DATA_H
#define JSTD_LANG_UTILS_AUDIO_WAV_DATA_H

#include <allocators/base_allocator.hpp>
#include <cpp/lang/io/istream.hpp>
#include <cstdint>

namespace jstd
{

/**
 * 0…3 (4 байта)	chunkId	Содержит символы "RIFF" в ASCII кодировке 0x52494646. Является началом RIFF-цепочки.
 * 4…7 (4 байта)	chunkSize	Это оставшийся размер цепочки, начиная с этой позиции. Иначе говоря, это размер файла минус 8, то есть, исключены поля chunkId и chunkSize.
 * 8…11 (4 байта)	format	Содержит символы "WAVE" 0x57415645
 * 12…15 (4 байта)	subchunk1Id	Содержит символы "fmt " 0x666d7420
 * 16…19 (4 байта)	subchunk1Size	16 для формата PCM. Это оставшийся размер подцепочки, начиная с этой позиции.
 * 
 * 20…21 (2 байта)	audioFormat	Аудио формат, список допустипых форматов. Для PCM = 1 (то есть, Линейное квантование). Значения, отличающиеся от 1, обозначают некоторый формат сжатия.
 * 22…23 (2 байта)	numChannels	Количество каналов. Моно = 1, Стерео = 2 и т.д.
 * 
 * 
 * 
 * 24…27 (4 байта)	sampleRate	Частота дискретизации. 8000 Гц, 44100 Гц и т.д.
 * 28…31 (4 байта)	byteRate	Количество байт, переданных за секунду воспроизведения.
 * 32…33 (2 байта)	blockAlign	Количество байт для одного сэмпла, включая все каналы.
 * 
 * 34…35 (2 байта)	bitsPerSample	Количество бит в сэмпле. Так называемая "глубина" или точность звучания. 8 бит, 16 бит и т.д.
 * 36…39 (4 байта)	subchunk2Id	Содержит символы "data" 0x64617461
 * 40…43 (4 байта)	subchunk2Size	Количество байт в области данных.
 * 44…	data	Непосредственно WAV-данные.
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
    void load_from(/*!non null!*/istream* in);
    void cleanup();
public:
    wav_data();
    /**
     * @param path
     *      Путь к файлу .wav
     *      Строка должная быть в формате UTF-8.
     * 
     * @param allocator
     *      Распределитель памяти, выделяющий память под данные.
     */
    wav_data(const char* path, tca::base_allocator* allocator);
    wav_data(istream* in, tca::base_allocator* allocator);
    ~wav_data();
    static const int32_t TO_STRING_MIN_BUFFER_SIZE = 256 + 128;
    int32_t to_string(char buf[], int32_t bufsize) const;
    
    const char* get_data() const;
    int32_t get_format() const;
    int32_t get_sample_rate() const;
    int32_t get_byte_rate() const;
    uint32_t get_length() const;
    int16_t get_num_channels() const;
    int16_t get_bits_per_sample() const;
};

}

#endif//JSTD_LANG_UTILS_AUDIO_WAV_DATA_H