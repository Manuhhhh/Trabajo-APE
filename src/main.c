#include <stdio.h>
#include <stdlib.h>
#include "wallet.h"

#define LINE_SIZE 256

enum {
    CRITERIA_BALANCE_ABOVE = 1,
    CRITERIA_OPERATIONS_IN_PERIOD,
    CRITERIA_MORE_OPERATIONS
};

typedef struct {
    int criteria;
    long long amount;
    Date *from;
    Date *to;
    size_t min_count;
} Filter;

/* ---------- Entrada ---------- */

/* Limpiar buffer: descarta lo que quedó en la entrada hasta el salto de línea, para no trabar la próxima lectura. */
void clear_buffer(void);

/* Leer texto: muestra el mensaje y lee una palabra; devuelve 1 si es válida. */
int read_string(const char *prompt, char *buf);

/* Leer entero: muestra el mensaje y lee un entero; devuelve 1 si es válido. */
int read_int(const char *prompt, int *value);

/* Leer monto: lee un monto en unidades de moneda y lo guarda en centavos; devuelve 1 si es válido. */
int read_amount(const char *prompt, long long *cents);

/* Leer fecha: lee una fecha en formato dd/mm/aaaa; si se presiona Enter usa la fecha de hoy. Devuelve 1 si es válida y deja en *d una fecha que el llamador debe liberar. */
int read_date(const char *prompt, Date **d);

/* ---------- Salida ---------- */

/* Mostrar monto: imprime un monto en centavos con formato $pesos.centavos. */
void print_amount(long long cents);

/* Mostrar usuario: imprime el id, nombre, saldo y cantidad de operaciones de un usuario. */
void print_user(const User *user);

/* Mostrar historial: imprime todas las operaciones de un usuario en orden de registro. */
void print_history(const User *user);

/* Mostrar resultado: imprime el mensaje correspondiente a un resultado de operación. */
void show_result(Result result);

/* ---------- Listados por criterio ---------- */

/* Tiene operación en período: indica si el usuario tiene alguna operación entre from y to, ambas incluidas. */
int has_operation_in_period(const User *user, const Date *from, const Date *to);

/* Cumple filtro: indica si el usuario cumple el criterio del filtro. */
int matches_filter(const User *user, const Filter *filter);

/* ---------- Comandos del menú ---------- */

/* Mostrar menú: imprime las opciones disponibles del programa. */
void show_menu(void);

/* Comando registrar: pide el nombre y da de alta un usuario nuevo. */
void cmd_register(Wallet *wallet);

/* Comando buscar: muestra los datos y el historial de un usuario según su id. */
void cmd_find(const Wallet *wallet);

/* Comando eliminar: quita un usuario según su id, si su saldo es cero. */
void cmd_delete(Wallet *wallet);

/* Comando movimiento: pide id, monto y fecha, y registra un depósito o un retiro según is_deposit. */
void cmd_movement(Wallet *wallet, int is_deposit);

/* Comando saldo: muestra el saldo actual de un usuario según su id. */
void cmd_balance(const Wallet *wallet);

/* Comando listar por criterio: pide un criterio y muestra los usuarios que lo cumplen. */
void cmd_list_by_criteria(const Wallet *wallet);

/* Comando listar todos: muestra todos los usuarios registrados en orden de id. */
void cmd_list_all(const Wallet *wallet);

/* Punto de entrada: muestra el menú en bucle y ejecuta la opción elegida hasta salir. */
int main(void);

void clear_buffer(void) {
    while (getchar() != '\n') {
    }
}

int read_string(const char *prompt, char *buf) {
    int ok;

    printf("%s", prompt);
    ok = scanf("%s", buf) == 1;
    clear_buffer();
    if (!ok) {
        printf("Entrada incorrecta, intente de nuevo.\n");
    }
    return ok;
}

int read_int(const char *prompt, int *value) {
    int ok;

    printf("%s", prompt);
    ok = scanf("%d", value) == 1;
    clear_buffer();
    if (!ok) {
        printf("Entrada incorrecta, intente de nuevo.\n");
    }
    return ok;
}

int read_amount(const char *prompt, long long *cents) {
    double value;
    int ok;

    printf("%s", prompt);
    ok = scanf("%lf", &value) == 1;
    clear_buffer();
    if (ok) {
        *cents = (long long)(value * 100);
    } else {
        printf("Entrada incorrecta, intente de nuevo.\n");
    }
    return ok;
}

int read_date(const char *prompt, Date **d) {
    int ch;
    int ok;
    int day = 0;
    int month = 0;
    int year = 0;

    printf("%s (dd/mm/aaaa, Enter = hoy): ", prompt);

    /* Si el primer carácter es el salto de línea se usa la fecha de hoy; si no, se devuelve al buffer */
    ch = getchar();
    if (ch == '\n') {
        *d = date_today();
        ok = 1;
    } else {
        ungetc(ch, stdin);
        ok = scanf("%d/%d/%d", &day, &month, &year) == 3;
        if (ok) {
            *d = date_create(day, month, year);
            ok = *d != NULL;
        }
        clear_buffer();
        if (!ok) {
            printf("Entrada incorrecta, intente de nuevo.\n");
        }
    }
    return ok;
}

void print_amount(long long cents) {
    printf("$%lld.%02lld", cents / 100, cents % 100);
}

void print_user(const User *user) {
    printf("[%d] %s | saldo: ", user_id(user), user_name(user));
    print_amount(user_balance(user));
    printf(" | operaciones: %zu\n", operation_list_count(user_operations(user)));
}

void print_history(const User *user) {
    const OperationList *operations = user_operations(user);
    size_t i;
    const char *type_name;

    for (i = 0; i < operation_list_count(operations); i++) {
        const Operation *op = operation_list_get(operations, i);
        const Date *date = operation_date(op);

        if (operation_type(op) == DEPOSIT) {
            type_name = "depósito";
        } else {
            type_name = "retiro";
        }
        printf("    #%d %02d/%02d/%04d %-10s ", operation_id(op), date_day(date), date_month(date),
               date_year(date), type_name);
        print_amount(operation_amount(op));
        putchar('\n');
    }
}

void show_result(Result result) {
    switch (result) {
    case RESULT_OK:
        printf("Operación realizada con éxito.\n");
        break;
    case RESULT_NOT_FOUND:
        printf("No existe un usuario con ese id.\n");
        break;
    case RESULT_INSUFFICIENT_FUNDS:
        printf("Saldo insuficiente para este retiro.\n");
        break;
    case RESULT_BALANCE_NOT_ZERO:
        printf("El usuario todavía tiene saldo. Retírelo antes de eliminarlo.\n");
        break;
    case RESULT_INVALID_AMOUNT:
        printf("El monto debe ser mayor que cero.\n");
        break;
    }
}

int has_operation_in_period(const User *user, const Date *from, const Date *to) {
    const OperationList *operations = user_operations(user);
    size_t i;
    int found = 0;

    for (i = 0; i < operation_list_count(operations) && !found; i++) {
        const Date *d = operation_date(operation_list_get(operations, i));

        found = date_compare(d, from) >= 0 && date_compare(d, to) <= 0;
    }
    return found;
}

int matches_filter(const User *user, const Filter *filter) {
    int matches = 0;

    switch (filter->criteria) {
    case CRITERIA_BALANCE_ABOVE:
        matches = user_balance(user) > filter->amount;
        break;
    case CRITERIA_OPERATIONS_IN_PERIOD:
        matches = has_operation_in_period(user, filter->from, filter->to);
        break;
    case CRITERIA_MORE_OPERATIONS:
        matches = operation_list_count(user_operations(user)) > filter->min_count;
        break;
    }
    return matches;
}

void show_menu(void) {
    printf("\n=== Billetera virtual ===\n");
    printf("1. Registrar usuario\n");
    printf("2. Buscar usuario por id (datos e historial)\n");
    printf("3. Eliminar usuario\n");
    printf("4. Depositar dinero\n");
    printf("5. Retirar dinero\n");
    printf("6. Consultar saldo\n");
    printf("7. Listar usuarios por criterio\n");
    printf("8. Listar todos los usuarios\n");
    printf("0. Salir\n");
}

void cmd_register(Wallet *wallet) {
    char name[LINE_SIZE];
    int id;

    if (read_string("Nombre: ", name)) {
        wallet_register(wallet, name, &id);
        printf("Usuario registrado con id %d.\n", id);
    }
}

void cmd_find(const Wallet *wallet) {
    int id;
    const User *user;

    if (read_int("Id de usuario: ", &id)) {
        user = wallet_find(wallet, id);
        if (user == NULL) {
            show_result(RESULT_NOT_FOUND);
        } else {
            print_user(user);
            print_history(user);
        }
    }
}

void cmd_delete(Wallet *wallet) {
    int id;

    if (read_int("Id de usuario: ", &id)) {
        show_result(wallet_remove(wallet, id));
    }
}

void cmd_movement(Wallet *wallet, int is_deposit) {
    int id;
    long long amount;
    Date *date;
    Result result;

    if (read_int("Id de usuario: ", &id) && read_amount("Monto: ", &amount) && read_date("Fecha", &date)) {
        if (is_deposit) {
            result = wallet_deposit(wallet, id, amount, date);
        } else {
            result = wallet_withdraw(wallet, id, amount, date);
        }
        date_destroy(date);
        show_result(result);
    }
}

void cmd_balance(const Wallet *wallet) {
    int id;
    const User *user;

    if (read_int("Id de usuario: ", &id)) {
        user = wallet_find(wallet, id);
        if (user == NULL) {
            show_result(RESULT_NOT_FOUND);
        } else {
            printf("Saldo de %s: ", user_name(user));
            print_amount(user_balance(user));
            putchar('\n');
        }
    }
}

void cmd_list_by_criteria(const Wallet *wallet) {
    Filter filter;
    const User **users;
    size_t i;
    size_t matches = 0;
    int value;
    int valid;

    filter.from = NULL;
    filter.to = NULL;
    printf("Criterios:\n");
    printf("  1. Saldo mayor que un monto\n");
    printf("  2. Con operaciones en un período\n");
    printf("  3. Con más de N operaciones\n");
    valid = read_int("Elija un criterio: ", &filter.criteria);
    if (valid) {
        switch (filter.criteria) {
        case CRITERIA_BALANCE_ABOVE:
            valid = read_amount("Monto: ", &filter.amount);
            break;
        case CRITERIA_OPERATIONS_IN_PERIOD:
            valid = read_date("Desde", &filter.from) && read_date("Hasta", &filter.to);
            if (valid && date_compare(filter.from, filter.to) > 0) {
                valid = 0;
                printf("Período inválido: la fecha desde es posterior a la fecha hasta.\n");
            }
            break;
        case CRITERIA_MORE_OPERATIONS:
            valid = read_int("Cantidad de operaciones (N): ", &value);
            filter.min_count = (size_t)value;
            break;
        default:
            valid = 0;
            printf("Criterio inválido.\n");
        }
    }
    if (valid) {
        users = wallet_users(wallet);
        for (i = 0; i < wallet_count(wallet); i++) {
            if (matches_filter(users[i], &filter)) {
                print_user(users[i]);
                matches++;
            }
        }
        free(users);
        printf("Usuarios encontrados: %zu\n", matches);
    }
    date_destroy(filter.from);
    date_destroy(filter.to);
}

void cmd_list_all(const Wallet *wallet) {
    const User **users;
    size_t i;

    if (wallet_count(wallet) == 0) {
        printf("No hay usuarios registrados.\n");
    } else {
        users = wallet_users(wallet);
        for (i = 0; i < wallet_count(wallet); i++) {
            print_user(users[i]);
        }
        free(users);
        printf("Total: %zu usuario(s).\n", wallet_count(wallet));
    }
}

int main(void) {
    Wallet *wallet = wallet_create();
    int option = -1;

    while (option != 0) {
        show_menu();
        if (!read_int("Opción: ", &option)) {
            option = -1;
        } else {
            switch (option) {
            case 1: cmd_register(wallet); break;
            case 2: cmd_find(wallet); break;
            case 3: cmd_delete(wallet); break;
            case 4: cmd_movement(wallet, 1); break;
            case 5: cmd_movement(wallet, 0); break;
            case 6: cmd_balance(wallet); break;
            case 7: cmd_list_by_criteria(wallet); break;
            case 8: cmd_list_all(wallet); break;
            case 0: break;
            default:
                printf("Opción inválida.\n");
            }
        }
    }
    wallet_destroy(wallet);
    printf("\n¡Hasta luego!\n");
    return 0;
}
