#ifndef WALLET_H
#define WALLET_H

#include <stddef.h>
#include "date.h"
#include "user.h"

/*
 * TAD Billetera: conjunto de usuarios indexados por id.
 * Implementado como un árbol AVL ordenado por id: búsqueda, alta y baja en
 * O(log n) en el peor caso, y recorrido in-order en orden de id.
 */
typedef struct Wallet Wallet;

typedef enum {
    RESULT_OK,
    RESULT_NOT_FOUND,
    RESULT_INSUFFICIENT_FUNDS,
    RESULT_BALANCE_NOT_ZERO,
    RESULT_INVALID_AMOUNT,
    RESULT_INVALID_DATE
} Result;

/* Acción a ejecutar sobre cada usuario durante un recorrido. */
typedef void (*Action)(const User *user, void *data);

Wallet *wallet_create(void);
void wallet_destroy(Wallet *wallet);
size_t wallet_count(const Wallet *wallet);

/* Registra un usuario con un id asignado por el sistema y lo devuelve en assigned_id. */
void wallet_register(Wallet *wallet, const char *name, int *assigned_id);

/* Devuelve NULL si no existe un usuario con ese id. */
const User *wallet_find(const Wallet *wallet, int id);

/* Solo se puede eliminar un usuario con saldo cero; su historial se elimina también. */
Result wallet_remove(Wallet *wallet, int id);

/* Montos en centavos. */
Result wallet_deposit(Wallet *wallet, int id, long long amount, Date date);
Result wallet_withdraw(Wallet *wallet, int id, long long amount, Date date);

/* Visita todos los usuarios en orden de id. */
void wallet_for_each(const Wallet *wallet, Action action, void *data);

#endif
