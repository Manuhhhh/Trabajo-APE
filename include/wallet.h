#ifndef WALLET_H
#define WALLET_H

#include <stddef.h>
#include "date.h"
#include "user.h"

/*
 * ADT Wallet: set of users indexed by id.
 * Implemented as an AVL tree ordered by id: search, insert and remove in
 * O(log n) worst case, and in-order traversal in id order.
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

/* Action to run on each user during a traversal. */
typedef void (*Action)(const User *user, void *data);

Wallet *wallet_create(void);
void wallet_destroy(Wallet *wallet);
size_t wallet_count(const Wallet *wallet);

/* Registers a user with an id assigned by the system; returns it in assigned_id. */
void wallet_register(Wallet *wallet, const char *name, int *assigned_id);

/* Returns NULL if no user with that id exists. */
const User *wallet_find(const Wallet *wallet, int id);

/* Only users with a zero balance can be removed; their history is removed too. */
Result wallet_remove(Wallet *wallet, int id);

/* Amounts in cents. */
Result wallet_deposit(Wallet *wallet, int id, long long amount, Date date);
Result wallet_withdraw(Wallet *wallet, int id, long long amount, Date date);

/* Visits every user in id order. */
void wallet_for_each(const Wallet *wallet, Action action, void *data);

#endif
