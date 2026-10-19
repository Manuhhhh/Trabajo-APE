#ifndef USER_H
#define USER_H

#include "operation_list.h"

#define MAX_NAME 50

typedef struct {
    int id;
    char name[MAX_NAME];
    long long balance;              /* en centavos */
    OperationList *operations;      /* historial en orden de registro */
} User;

#endif
