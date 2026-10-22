#ifndef DATE_H
#define DATE_H

/*
 * TAD Fecha: día, mes y año del calendario gregoriano (1900-2100).
 * Solo se crea con date_create, date_today o date_copy, así que toda Date existente es válida.
 */
typedef struct Date Date;

/* Devuelve NULL si la fecha no existe en el calendario. Se libera con date_destroy. */
Date *date_create(int day, int month, int year);

/* Fecha actual del sistema. Se libera con date_destroy. */
Date *date_today(void);

/* Copia independiente de la fecha. Se libera con date_destroy. */
Date *date_copy(const Date *date);

/* Libera la fecha; acepta NULL. */
void date_destroy(Date *date);

int date_day(const Date *date);
int date_month(const Date *date);
int date_year(const Date *date);

/* Devuelve <0, 0 o >0 si date es anterior, igual o posterior a other. */
int date_compare(const Date *date, const Date *other);

#endif
