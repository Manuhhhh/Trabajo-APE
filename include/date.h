#ifndef DATE_H
#define DATE_H

typedef struct {
    int day;
    int month;
    int year;
} Date;

/* Returns 1 if the date exists in the Gregorian calendar (1900-2100), 0 otherwise. */
int date_is_valid(Date d);

/* Returns <0, 0 or >0 if d is before, equal to or after other. */
int date_compare(Date d, Date other);

/* Current system date. */
Date date_today(void);

#endif
