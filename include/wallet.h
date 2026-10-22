#ifndef WALLET_H
#define WALLET_H

#include <stddef.h>
#include "date.h"
#include "user.h"

/*
 * TAD Billetera: conjunto de usuarios indexados por id, con las reglas para registrarlos,
 * eliminarlos y aplicarles depósitos y retiros. Asigna los ids de usuarios y de operaciones.
 */
typedef struct Wallet Wallet;

typedef enum {
    RESULT_OK,
    RESULT_NOT_FOUND,
    RESULT_INSUFFICIENT_FUNDS,
    RESULT_BALANCE_NOT_ZERO,
    RESULT_INVALID_AMOUNT
} Result;

Wallet *wallet_create(void);
void wallet_destroy(Wallet *wallet);
size_t wallet_count(const Wallet *wallet);

/* Registra un usuario con un id asignado por el sistema y lo devuelve en assigned_id. */
void wallet_register(Wallet *wallet, const char *name, int *assigned_id);

/* Devuelve NULL si no existe un usuario con ese id. */
const User *wallet_find(const Wallet *wallet, int id);

/* Solo se puede eliminar un usuario con saldo cero; su historial se elimina también. */
Result wallet_remove(Wallet *wallet, int id);

/* Montos en centavos. La operación guarda su propia copia de la fecha. */
Result wallet_deposit(Wallet *wallet, int id, long long amount, const Date *date);
Result wallet_withdraw(Wallet *wallet, int id, long long amount, const Date *date);

/*
 * Devuelve un arreglo nuevo con wallet_count(wallet) usuarios en orden de id.
 * El arreglo lo libera el llamador con free; los usuarios siguen siendo de la billetera.
 */
const User **wallet_users(const Wallet *wallet);

#endif
