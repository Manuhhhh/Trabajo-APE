#ifndef OPERATION_LIST_H
#define OPERATION_LIST_H

#include <stddef.h>
#include "operation.h"

/*
 * TAD Lista de operaciones: secuencia dinámica de operaciones en orden de registro.
 * Agregar al final en O(1) amortizado; acceso por posición en O(1).
 */
typedef struct OperationList OperationList;

OperationList *operation_list_create(void);
void operation_list_destroy(OperationList *list);

/* Agrega una operación al final de la lista. */
void operation_list_append(OperationList *list, Operation op);

size_t operation_list_count(const OperationList *list);

/* Devuelve NULL si pos está fuera de rango. */
const Operation *operation_list_get(const OperationList *list, size_t pos);

#endif
