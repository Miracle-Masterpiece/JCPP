#ifndef _JSTD_CPP_LANG_UTIL_DATE_H
#define _JSTD_CPP_LANG_UTIL_DATE_H

#include <ctime>
#include <cstdint>

namespace jstd {

/**
 * Класс для работы с датой и временем.
 * Предоставляет функциональность для создания, сравнения, форматирования и манипуляции датами.
 * Внутренне использует структуру `time_t` для хранения времени и `tm` для локального представления.
 * 
 * @note Класс поддерживает копирование, перемещение и сравнение дат.
 *       Также предоставляет методы для получения отдельных компонентов даты (день, месяц, год и т.д.).
 */
class date { 
    time_t _time;    // Время в формате time_t (секунды с начала Unix-эпохи).
    tm _localTime;   // Локальное представление времени в структуре tm.
public:
    enum month {
        JANUARY = 1,   // Январь
        FEBRUARY,      // Февраль
        MARCH,         // Март
        APRIL,         // Апрель
        MAY,           // Май
        JUNE,          // Июнь
        JULY,          // Июль
        AUGUST,        // Август
        SEPTEMBER,     // Сентябрь
        OCTOBER,       // Октябрь
        NOVEMBER,      // Ноябрь
        DECEMBER       // Декабрь
    };
    
    /**
     * Конструктор, создающий объект даты с указанными компонентами.
     * 
     * @param day    День месяца (1-31).
     * @param month  Месяц (1-12).
     * @param year   Год (например, 2023).
     * @param second Секунды (0-59).
     * @param minute Минуты (0-59).
     * @param hour   Часы (0-23).
     */
    date(int32_t day = 1, int32_t month = 1, int32_t year = 0, int32_t second = 0, int32_t minute = 0, int32_t hour = 0);

    /**
     * Конструктор, создающий объект даты из времени в формате `time_t`.
     * 
     * @param date Время в формате `time_t` (секунды с начала Unix-эпохи).
     */
    date(time_t date);

    /**
     * Конструктор копирования.
     * 
     * @param other Копируемый объект `date`.
     */
    date(const date& other);

    /**
     * Конструктор перемещения.
     * 
     * @param other Перемещаемый объект `date`.
     */
    date(date&& other);

    /**
     * Оператор присваивания копированием.
     * 
     * @param other Копируемый объект `date`.
     * @return Ссылка на текущий объект.
     */
    date& operator=(const date& other);

    /**
     * Оператор присваивания перемещением.
     * 
     * @param other Перемещаемый объект `date`.
     * @return Ссылка на текущий объект.
     */
    date& operator=(date&& other);

    /**
     * Деструктор.
     */
    ~date();

    /**
     * Возвращает текущую дату и время.
     * 
     * @return Объект `date`, представляющий текущий момент времени.
     */
    static date now();

    static const int TO_STRING_MIN_BUFFER_SIZE = 48;
    /**
     * Преобразует дату в строковое представление и записывает его в буфер.
     * 
     * @param buf     
     *      Буфер для записи строки.
     * 
     * @param bufsize 
     *      Размер буфера.
     * 
     * @return 
     *      Количество записанных символов или -1 в случае ошибки.
     */
    int32_t to_string(char buf[], int32_t bufsize) const;

    /**
     * Сравнивает текущую дату с другой датой на равенство.
     * 
     * @param other Дата для сравнения.
     * @return true, если даты равны, иначе false.
     */
    bool equals(const date& other) const;

    /**
     * Возвращает хэш-код даты.
     * 
     * @return Хэш-код в формате int64_t.
     */
    uint64_t hashcode() const;

    /**
     * Сравнивает текущую дату с другой датой.
     * 
     * @param other Дата для сравнения.
     * @return 0, если даты равны; отрицательное число, если текущая дата меньше; положительное число, если больше.
     */
    int32_t compare_to(const date& other) const;

    /**
     * Оператор сравнения на равенство.
     * 
     * @param other Дата для сравнения.
     * @return true, если даты равны, иначе false.
     */
    bool operator==(const date& other) const;

    /**
     * Оператор сравнения на неравенство.
     * 
     * @param other Дата для сравнения.
     * @return true, если даты не равны, иначе false.
     */
    bool operator!=(const date& other) const;

    /**
     * Оператор сравнения "больше".
     * 
     * @param other Дата для сравнения.
     * @return true, если текущая дата больше, иначе false.
     */
    bool operator>(const date& other) const;

    /**
     * Оператор сравнения "меньше".
     * 
     * @param other Дата для сравнения.
     * @return true, если текущая дата меньше, иначе false.
     */
    bool operator<(const date& other) const;

    /**
     * Оператор сравнения "больше или равно".
     * 
     * @param other Дата для сравнения.
     * @return true, если текущая дата больше или равна, иначе false.
     */
    bool operator>=(const date& other) const;

    /**
     * Оператор сравнения "меньше или равно".
     * 
     * @param other Дата для сравнения.
     * @return true, если текущая дата меньше или равна, иначе false.
     */
    bool operator<=(const date& other) const;

    /**
     * Устанавливает время в формате `time_t`.
     * 
     * @param date Время в формате `time_t`.
     */
    void set_time(time_t date);

    /**
     * Возвращает время в формате `time_t`.
     * 
     * @return Время в формате `time_t`.
     */
    time_t get_time() const;

    /**
     * Возвращает день месяца.
     * 
     * @return День месяца (1-31).
     */
    int32_t get_day() const;

    /**
     * Возвращает месяц.
     * 
     * @return Месяц (1-12).
     */
    int32_t get_month() const;

    /**
     * Возвращает год.
     * 
     * @return Год (например, 2023).
     */
    int32_t get_year() const;

    /**
     * Возвращает час.
     * 
     * @return Час (0-23).
     */
    int32_t get_hour() const;

    /**
     * Возвращает минуты.
     * 
     * @return Минуты (0-59).
     */
    int32_t get_minute() const;

    /**
     * Возвращает секунды.
     * 
     * @return Секунды (0-59).
     */
    int32_t get_second() const;

    /**
     * Возвращает номер недели в году.
     * 
     * @return Номер недели (1-53).
     */
    int32_t get_week() const;
};

template<typename T>
struct hash_for;

template<typename T>
struct equal_to;

template<>
struct equal_to<date> {
    bool operator() (const date& i1, const date& i2) {
        return i1.equals(i2);
    }
};

template<>
struct hash_for<date> {
    uint64_t operator() (const date& i) {
        return i.hashcode();
    }
};

}
#endif//_JSTD_CPP_LANG_UTIL_DATE_H