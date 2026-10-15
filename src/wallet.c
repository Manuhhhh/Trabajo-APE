#include <stdio.h>
#include <stdlib.h>
#include "wallet.h"

typedef struct Node {
    User user;
    int height;
    struct Node *left;
    struct Node *right;
} Node;

struct Wallet {
    Node *root;
    size_t count;
    int next_user_id;
    int next_operation_id;
};

/* ---------- AVL balancing ---------- */

/* Altura: devuelve la altura de un nodo, o 0 si el nodo es NULL. */
int height(const Node *n);

/* Máximo: devuelve el mayor de dos enteros. */
int max_int(int a, int b);

/* Actualizar altura: recalcula la altura del nodo a partir de la de sus hijos. */
void update_height(Node *n);

/* Factor de balance: diferencia de altura entre el subárbol izquierdo y el derecho. */
int balance_factor(const Node *n);

/* Rotar a la derecha: reequilibra el subárbol girando hacia la derecha y devuelve la nueva raíz. */
Node *rotate_right(Node *y);

/* Rotar a la izquierda: reequilibra el subárbol girando hacia la izquierda y devuelve la nueva raíz. */
Node *rotate_left(Node *x);

/* Rebalancear: recalcula la altura del nodo y lo rebalancea si su factor está fuera de [-1, 1]. Devuelve la nueva raíz del subárbol. */
Node *rebalance(Node *n);

/* ---------- Recursive tree operations ---------- */

/* Insertar nodo: inserta el nodo nuevo ordenado por id y devuelve la raíz del subárbol. */
Node *insert_node(Node *n, Node *new_node);

/* Buscar nodo: devuelve el nodo con ese id, o NULL si no existe. */
Node *find_node(Node *n, int id);

/* Quitar mínimo: desconecta el nodo más chico del subárbol, lo devuelve en *min y retorna la nueva raíz. */
Node *remove_min(Node *n, Node **min);

/* Quitar nodo: desconecta el nodo con ese id, lo devuelve en *removed y retorna la nueva raíz del subárbol. */
Node *remove_node(Node *n, int id, Node **removed);

/* Recorrer nodos: visita los usuarios del subárbol en orden de id, aplicando action a cada uno. */
void for_each_node(const Node *n, Action action, void *data);

/* Destruir nodos: libera todos los nodos del subárbol junto con sus historiales. */
void destroy_nodes(Node *n);

/* ---------- Public interface ---------- */

/* Registrar movimiento: valida un depósito o retiro y lo agrega al historial del usuario. */
Result record_movement(Wallet *wallet, int id, OperationType type, long long amount, Date date);

/* Crear billetera: crea una billetera vacía. */
Wallet *wallet_create(void);

/* Destruir billetera: libera la billetera con todos sus usuarios. */
void wallet_destroy(Wallet *wallet);

/* Contar usuarios: devuelve la cantidad de usuarios registrados. */
size_t wallet_count(const Wallet *wallet);

/* Registrar usuario: da de alta un usuario con un id asignado por el sistema y lo devuelve en assigned_id. */
void wallet_register(Wallet *wallet, const char *name, int *assigned_id);

/* Buscar usuario: devuelve el usuario con ese id, o NULL si no existe. */
const User *wallet_find(const Wallet *wallet, int id);

/* Eliminar usuario: quita un usuario con saldo cero, junto con su historial. */
Result wallet_remove(Wallet *wallet, int id);

/* Depositar: suma un monto (en centavos) al saldo del usuario. */
Result wallet_deposit(Wallet *wallet, int id, long long amount, Date date);

/* Retirar: resta un monto (en centavos) del saldo del usuario. */
Result wallet_withdraw(Wallet *wallet, int id, long long amount, Date date);

/* Recorrer usuarios: aplica una acción a cada usuario en orden de id. */
void wallet_for_each(const Wallet *wallet, Action action, void *data);

int height(const Node *n) {
    return n ? n->height : 0;
}

int max_int(int a, int b) {
    return a > b ? a : b;
}

void update_height(Node *n) {
    n->height = 1 + max_int(height(n->left), height(n->right));
}

int balance_factor(const Node *n) {
    return n ? height(n->left) - height(n->right) : 0;
}

Node *rotate_right(Node *y) {
    Node *x = y->left;

    y->left = x->right;
    x->right = y;
    update_height(y);
    update_height(x);
    return x;
}

Node *rotate_left(Node *x) {
    Node *y = x->right;

    x->right = y->left;
    y->left = x;
    update_height(x);
    update_height(y);
    return y;
}

Node *rebalance(Node *n) {
    int balance;

    update_height(n);
    balance = balance_factor(n);
    if (balance > 1) {
        if (balance_factor(n->left) < 0) {
            n->left = rotate_left(n->left);
        }
        n = rotate_right(n);
    } else if (balance < -1) {
        if (balance_factor(n->right) > 0) {
            n->right = rotate_right(n->right);
        }
        n = rotate_left(n);
    }
    return n;
}

Node *insert_node(Node *n, Node *new_node) {
    if (!n) {
        n = new_node;
    } else {
        if (new_node->user.id < n->user.id) {
            n->left = insert_node(n->left, new_node);
        } else {
            n->right = insert_node(n->right, new_node);
        }
        n = rebalance(n);
    }
    return n;
}

Node *find_node(Node *n, int id) {
    Node *found = n;

    if (n && n->user.id != id) {
        found = find_node(id < n->user.id ? n->left : n->right, id);
    }
    return found;
}

Node *remove_min(Node *n, Node **min) {
    if (!n->left) {
        *min = n;
        n = n->right;
    } else {
        n->left = remove_min(n->left, min);
        n = rebalance(n);
    }
    return n;
}

Node *remove_node(Node *n, int id, Node **removed) {
    Node *result = n;

    if (n) {
        if (id < n->user.id) {
            n->left = remove_node(n->left, id, removed);
            result = rebalance(n);
        } else if (id > n->user.id) {
            n->right = remove_node(n->right, id, removed);
            result = rebalance(n);
        } else {
            Node *successor = NULL;
            Node *rest;

            *removed = n;
            if (!n->left) {
                result = n->right;
            } else if (!n->right) {
                result = n->left;
            } else {
                rest = remove_min(n->right, &successor);
                successor->left = n->left;
                successor->right = rest;
                result = rebalance(successor);
            }
        }
    }
    return result;
}

void for_each_node(const Node *n, Action action, void *data) {
    if (n) {
        for_each_node(n->left, action, data);
        action(&n->user, data);
        for_each_node(n->right, action, data);
    }
}

void destroy_nodes(Node *n) {
    if (n) {
        destroy_nodes(n->left);
        destroy_nodes(n->right);
        operation_list_destroy(n->user.operations);
        free(n);
    }
}

Wallet *wallet_create(void) {
    Wallet *wallet = malloc(sizeof *wallet);

    wallet->root = NULL;
    wallet->count = 0;
    wallet->next_user_id = 1;
    wallet->next_operation_id = 1;
    return wallet;
}

void wallet_destroy(Wallet *wallet) {
    if (wallet) {
        destroy_nodes(wallet->root);
        free(wallet);
    }
}

size_t wallet_count(const Wallet *wallet) {
    return wallet->count;
}

void wallet_register(Wallet *wallet, const char *name, int *assigned_id) {
    Node *node = calloc(1, sizeof *node);

    node->user.operations = operation_list_create();
    node->user.id = wallet->next_user_id++;
    snprintf(node->user.name, sizeof node->user.name, "%s", name);
    node->height = 1;

    wallet->root = insert_node(wallet->root, node);
    wallet->count++;
    *assigned_id = node->user.id;
}

const User *wallet_find(const Wallet *wallet, int id) {
    const Node *n = find_node(wallet->root, id);

    return n ? &n->user : NULL;
}

Result wallet_remove(Wallet *wallet, int id) {
    Node *n = find_node(wallet->root, id);
    Node *removed = NULL;
    Result result = RESULT_NOT_FOUND;

    if (n) {
        if (n->user.balance != 0) {
            result = RESULT_BALANCE_NOT_ZERO;
        } else {
            wallet->root = remove_node(wallet->root, id, &removed);
            operation_list_destroy(removed->user.operations);
            free(removed);
            wallet->count--;
            result = RESULT_OK;
        }
    }
    return result;
}

Result record_movement(Wallet *wallet, int id, OperationType type, long long amount, Date date) {
    Node *n = find_node(wallet->root, id);
    Operation op;
    Result result = RESULT_OK;

    if (amount <= 0) {
        result = RESULT_INVALID_AMOUNT;
    } else if (!date_is_valid(date)) {
        result = RESULT_INVALID_DATE;
    } else if (!n) {
        result = RESULT_NOT_FOUND;
    } else if (type == WITHDRAWAL && amount > n->user.balance) {
        result = RESULT_INSUFFICIENT_FUNDS;
    } else {
        op.id = wallet->next_operation_id;
        op.type = type;
        op.amount = amount;
        op.date = date;
        operation_list_append(n->user.operations, op);
        wallet->next_operation_id++;
        n->user.balance += type == DEPOSIT ? amount : -amount;
    }
    return result;
}

Result wallet_deposit(Wallet *wallet, int id, long long amount, Date date) {
    return record_movement(wallet, id, DEPOSIT, amount, date);
}

Result wallet_withdraw(Wallet *wallet, int id, long long amount, Date date) {
    return record_movement(wallet, id, WITHDRAWAL, amount, date);
}

void wallet_for_each(const Wallet *wallet, Action action, void *data) {
    for_each_node(wallet->root, action, data);
}
