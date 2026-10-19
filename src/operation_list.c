#include <stdlib.h>
#include "operation_list.h"

#define INITIAL_CAPACITY 4

struct OperationList {
    Operation *data;
    size_t count;
    size_t capacity;
};

/* Crear lista: reserva una lista de operaciones vacía. */
OperationList *operation_list_create(void);

/* Destruir lista: libera la lista junto con sus operaciones. */
void operation_list_destroy(OperationList *list);

/* Agregar operación: añade una operación al final de la lista, ampliando la memoria si hace falta. */
void operation_list_append(OperationList *list, Operation op);

/* Cantidad de operaciones: devuelve cuántas operaciones tiene la lista. */
size_t operation_list_count(const OperationList *list);

/* Obtener operación: devuelve la operación en la posición indicada, o NULL si está fuera de rango. */
const Operation *operation_list_get(const OperationList *list, size_t pos);

OperationList *operation_list_create(void) {
    OperationList *list = malloc(sizeof(OperationList));

    list->data = malloc(INITIAL_CAPACITY * sizeof(Operation));
    list->count = 0;
    list->capacity = INITIAL_CAPACITY;
    return list;
}

void operation_list_destroy(OperationList *list) {
    free(list->data);
    free(list);
}

void operation_list_append(OperationList *list, Operation op) {
    if (list->count == list->capacity) {
        list->capacity = list->capacity * 2;
        list->data = realloc(list->data, list->capacity * sizeof(Operation));
    }
    list->data[list->count] = op;
    list->count++;
}

size_t operation_list_count(const OperationList *list) {
    return list->count;
}

const Operation *operation_list_get(const OperationList *list, size_t pos) {
    const Operation *result = NULL;

    if (pos < list->count) {
        result = &list->data[pos];
    }
    return result;
}
