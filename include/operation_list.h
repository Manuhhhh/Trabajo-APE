#ifndef OPERATION_LIST_H
#define OPERATION_LIST_H

#include <stddef.h>
#include "operation.h"

/*
 * ADT Operation list: dynamic sequence of operations in registration order.
 * Append at the end in amortized O(1); access by position in O(1).
 */
typedef struct OperationList OperationList;

OperationList *operation_list_create(void);
void operation_list_destroy(OperationList *list);

/* Appends an operation at the end of the list. */
void operation_list_append(OperationList *list, Operation op);

size_t operation_list_count(const OperationList *list);

/* Returns NULL if pos is out of range. */
const Operation *operation_list_get(const OperationList *list, size_t pos);

#endif
