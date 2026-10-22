#include <stdlib.h>
#include "operation.h"
#include "user_tree.h"
#include "wallet.h"

struct Wallet {
    UserTree *users;        /* NULL si no hay usuarios */
    size_t count;
    int next_user_id;       /* solo crece: los ids eliminados no se reutilizan */
    int next_operation_id;  /* id global de operaciones, compartido por todos los usuarios */
};

/* Registrar movimiento: valida un depósito o retiro y lo agrega al historial del usuario. */
static Result record_movement(Wallet *wallet, int id, OperationType type, long long amount, const Date *date);

static Result record_movement(Wallet *wallet, int id, OperationType type, long long amount, const Date *date) {
    User *user = user_tree_find(wallet->users, id);
    Result result = RESULT_OK;

    if (amount <= 0) {
        result = RESULT_INVALID_AMOUNT;
    } else if (user == NULL) {
        result = RESULT_NOT_FOUND;
    } else if (type == WITHDRAWAL && amount > user_balance(user)) {
        result = RESULT_INSUFFICIENT_FUNDS;
    } else {
        user_record(user, operation_create(wallet->next_operation_id, type, amount, date));
        wallet->next_operation_id++;
    }
    return result;
}

Wallet *wallet_create(void) {
    Wallet *wallet = malloc(sizeof(Wallet));

    wallet->users = NULL;
    wallet->count = 0;
    wallet->next_user_id = 1;
    wallet->next_operation_id = 1;
    return wallet;
}

void wallet_destroy(Wallet *wallet) {
    if (wallet != NULL) {
        user_tree_destroy(wallet->users);
        free(wallet);
    }
}

size_t wallet_count(const Wallet *wallet) {
    return wallet->count;
}

void wallet_register(Wallet *wallet, const char *name, int *assigned_id) {
    User *user = user_create(wallet->next_user_id, name);

    wallet->next_user_id++;
    /* La raíz puede cambiar al insertar un nuevo usuario */
    wallet->users = user_tree_insert(wallet->users, user);
    wallet->count++;
    *assigned_id = user_id(user);
}

const User *wallet_find(const Wallet *wallet, int id) {
    return user_tree_find(wallet->users, id);
}

Result wallet_remove(Wallet *wallet, int id) {
    User *user = user_tree_find(wallet->users, id);
    User *removed = NULL;
    Result result = RESULT_NOT_FOUND;

    if (user != NULL) {
        if (user_balance(user) != 0) {
            result = RESULT_BALANCE_NOT_ZERO;
        } else {
            /* user_tree_remove devuelve la nueva raíz y deja al usuario en removed para liberarlo acá */
            wallet->users = user_tree_remove(wallet->users, id, &removed);
            user_destroy(removed);
            wallet->count--;
            result = RESULT_OK;
        }
    }
    return result;
}

Result wallet_deposit(Wallet *wallet, int id, long long amount, const Date *date) {
    return record_movement(wallet, id, DEPOSIT, amount, date);
}

Result wallet_withdraw(Wallet *wallet, int id, long long amount, const Date *date) {
    return record_movement(wallet, id, WITHDRAWAL, amount, date);
}

const User **wallet_users(const Wallet *wallet) {
    const User **users = malloc(wallet->count * sizeof(const User *));

    user_tree_to_array(wallet->users, users);
    return users;
}
