#ifndef DATE_H
#define DATE_H

typedef struct {
    int day;
    int month;
    int year;
} Date;

/* Devuelve 1 si la fecha existe en el calendario gregoriano (1900-2100), 0 si no. */
int date_is_valid(Date d);

/* Devuelve <0, 0 o >0 si d es anterior, igual o posterior a other. */
int date_compare(Date d, Date other);

/* Fecha actual del sistema. */
Date date_today(void);

#endif
