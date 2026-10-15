#ifndef USER_H
#define USER_H

#include "operation_list.h"

#define MAX_NAME 50

typedef struct {
    int id;
    char name[MAX_NAME];
    long long balance;              /* in cents */
    OperationList *operations;      /* history in registration order */
} User;

#endif
