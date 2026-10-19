#ifndef OPERATION_H
#define OPERATION_H

#include "date.h"

typedef enum { DEPOSIT, WITHDRAWAL } OperationType;

typedef struct {
    int id;
    OperationType type;
    long long amount;   /* en centavos */
    Date date;
} Operation;

#endif
