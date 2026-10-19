#include <time.h>
#include "date.h"

/* Días del mes: devuelve cuántos días tiene el mes indicado, según si el año es bisiesto. */
int days_in_month(int month, int year);

/* Clave de orden: convierte la fecha en un número aaaammdd para poder compararla. */
long key(Date d);

/* Validar fecha: indica si la fecha existe en el calendario gregoriano (años 1900 a 2100). */
int date_is_valid(Date d);

/* Comparar fechas: devuelve <0, 0 o >0 si d es anterior, igual o posterior a other. */
int date_compare(Date d, Date other);

/* Fecha de hoy: devuelve la fecha actual del sistema. */
Date date_today(void);

int days_in_month(int month, int year) {
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    int result = days[month - 1];

    if (month == 2 && leap) {
        result = 29;
    }
    return result;
}

int date_is_valid(Date d) {
    return d.year >= 1900 && d.year <= 2100 &&
           d.month >= 1 && d.month <= 12 &&
           d.day >= 1 && d.day <= days_in_month(d.month, d.year);
}

long key(Date d) {
    return d.year * 10000L + d.month * 100L + d.day;
}

int date_compare(Date d, Date other) {
    long a = key(d);
    long b = key(other);
    int result = 0;

    if (a < b) {
        result = -1;
    } else if (a > b) {
        result = 1;
    }
    return result;
}

Date date_today(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    Date today;

    today.day = t->tm_mday;
    today.month = t->tm_mon + 1;
    today.year = t->tm_year + 1900;
    return today;
}
