#include <stdlib.h>
#include <time.h>
#include "date.h"

struct Date {
    int day;
    int month;
    int year;
};

/* Días del mes: devuelve cuántos días tiene el mes indicado, según si el año es bisiesto. */
static int days_in_month(int month, int year);

/* Clave de orden: convierte la fecha en un número aaaammdd para poder compararla. */
static long key(const Date *date);

/* Validar fecha: indica si la fecha existe en el calendario gregoriano (años 1900 a 2100). */
static int is_valid(int day, int month, int year);

static int days_in_month(int month, int year) {
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    int result = days[month - 1];

    if (month == 2 && leap) {
        result = 29;
    }
    return result;
}

static int is_valid(int day, int month, int year) {
    return year >= 1900 && year <= 2100 &&
           month >= 1 && month <= 12 &&
           day >= 1 && day <= days_in_month(month, year);
}

static long key(const Date *date) {
    return date->year * 10000L + date->month * 100L + date->day;
}

Date *date_create(int day, int month, int year) {
    Date *date = NULL;

    if (is_valid(day, month, year)) {
        date = malloc(sizeof(Date));
        date->day = day;
        date->month = month;
        date->year = year;
    }
    return date;
}

Date *date_today(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    return date_create(t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
}

Date *date_copy(const Date *date) {
    return date_create(date->day, date->month, date->year);
}

void date_destroy(Date *date) {
    free(date);
}

int date_day(const Date *date) {
    return date->day;
}

int date_month(const Date *date) {
    return date->month;
}

int date_year(const Date *date) {
    return date->year;
}

int date_compare(const Date *date, const Date *other) {
    long a = key(date);
    long b = key(other);
    int result = 0;

    if (a < b) {
        result = -1;
    } else if (a > b) {
        result = 1;
    }
    return result;
}
