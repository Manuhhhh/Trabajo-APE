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
    Date from;
    Date to;
    size_t min_count;
    size_t matches;
} Filter;

/* ---------- Input ---------- */

/* Leer línea: lee una línea de la entrada sin el salto de línea; si la entrada termina, cierra el programa. */
void read_line(char *buf);

/* Leer entero: muestra el mensaje y lee un entero; devuelve 1 si la entrada es válida. */
int read_long(const char *prompt, long *value);

/* Leer id: pide un id de usuario positivo; devuelve 1 si es válido. */
int read_id(int *id);

/* Leer monto: lee un monto en unidades de moneda y lo guarda en centavos; devuelve 1 si es válido. */
int read_amount(const char *prompt, long long *cents);

/* Leer fecha: lee una fecha en formato dd/mm/yyyy; si se presiona Enter usa la fecha de hoy. Devuelve 1 si es válida. */
int read_date(const char *prompt, Date *d);

/* ---------- Output ---------- */

/* Mostrar monto: imprime un monto en centavos con formato $pesos.centavos. */
void print_amount(long long cents);

/* Mostrar usuario: imprime el id, nombre, saldo y cantidad de operaciones de un usuario. */
void print_user(const User *user);

/* Mostrar historial: imprime todas las operaciones de un usuario en orden de registro. */
void print_history(const User *user);

/* Mostrar resultado: imprime el mensaje correspondiente a un resultado de operación. */
void show_result(Result result);

/* Mostrar usuario (acción): adaptador de print_user para usarlo en recorridos de la billetera. */
void print_user_action(const User *user, void *data);

/* ---------- Listings by criteria ---------- */

/* Tiene operación en período: indica si el usuario tiene alguna operación entre from y to, ambas incluidas. */
int has_operation_in_period(const User *user, Date from, Date to);

/* Aplicar filtro: muestra el usuario si cumple el criterio del filtro y cuenta las coincidencias. */
void apply_filter(const User *user, void *data);

/* ---------- Menu commands ---------- */

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

void read_line(char *buf) {
    int ch;
    int items;

    /* Leer el texto hasta el salto de línea */
    buf[0] = '\0';
    items = scanf("%255[^\n]", buf);

    /* Si no se leyó nada y la entrada terminó, cerrar el programa */
    if (items != 1 && feof(stdin)) {
        printf("\nFin de la entrada.\n");
        exit(EXIT_SUCCESS);
    }

    /*
     * Sacar del buffer de entrada lo que scanf dejó: el salto de línea o el resto
     * de una línea más larga. Si no se sacan, la próxima lectura falla sin avanzar.
     */
    ch = getchar();
    while (ch != '\n' && !feof(stdin)) {
        ch = getchar();
    }
}

int read_long(const char *prompt, long *value) {
    char line[LINE_SIZE];
    char *end;

    printf("%s", prompt);
    read_line(line);
    *value = strtol(line, &end, 10);
    return end != line && *end == '\0';
}

int read_id(int *id) {
    long value;
    int ok = read_long("Id de usuario: ", &value) && value > 0;

    if (ok) {
        *id = (int)value;
    }
    return ok;
}

/* Reads an amount in currency units and stores it in cents. */
int read_amount(const char *prompt, long long *cents) {
    char line[LINE_SIZE];
    char *end;
    double value;
    int ok;

    printf("%s", prompt);
    read_line(line);
    value = strtod(line, &end);
    ok = end != line && *end == '\0' && value > 0 && value < 1e9;
    if (ok) {
        *cents = (long long)(value * 100 + 0.5);
    }
    return ok;
}

/* Reads a date as dd/mm/yyyy; pressing Enter uses today's date. */
int read_date(const char *prompt, Date *d) {
    char line[LINE_SIZE];
    int ok;

    printf("%s (dd/mm/aaaa, Enter = hoy): ", prompt);
    read_line(line);
    if (line[0] == '\0') {
        *d = date_today();
        ok = 1;
    } else {
        ok = sscanf(line, "%d/%d/%d", &d->day, &d->month, &d->year) == 3 && date_is_valid(*d);
    }
    return ok;
}

void print_amount(long long cents) {
    printf("$%lld.%02lld", cents / 100, cents % 100);
}

void print_user(const User *user) {
    printf("[%d] %s | saldo: ", user->id, user->name);
    print_amount(user->balance);
    printf(" | operaciones: %zu\n", operation_list_count(user->operations));
}

void print_history(const User *user) {
    size_t i;

    for (i = 0; i < operation_list_count(user->operations); i++) {
        const Operation *op = operation_list_get(user->operations, i);

        printf("    #%d %02d/%02d/%04d %-10s ", op->id, op->date.day, op->date.month,
               op->date.year, op->type == DEPOSIT ? "depósito" : "retiro");
        print_amount(op->amount);
        putchar('\n');
    }
}

void show_result(Result result) {
    const char *const messages[] = {
        [RESULT_OK] = "Operación realizada con éxito.",
        [RESULT_NOT_FOUND] = "No existe un usuario con ese id.",
        [RESULT_INSUFFICIENT_FUNDS] = "Saldo insuficiente para este retiro.",
        [RESULT_BALANCE_NOT_ZERO] = "El usuario todavía tiene saldo. Retírelo antes de eliminarlo.",
        [RESULT_INVALID_AMOUNT] = "El monto debe ser mayor que cero.",
        [RESULT_INVALID_DATE] = "La fecha no es válida.",
    };

    printf("%s\n", messages[result]);
}

void print_user_action(const User *user, void *data) {
    (void)data;
    print_user(user);
}

int has_operation_in_period(const User *user, Date from, Date to) {
    size_t i;
    int found = 0;

    for (i = 0; i < operation_list_count(user->operations) && !found; i++) {
        const Date *d = &operation_list_get(user->operations, i)->date;

        found = date_compare(*d, from) >= 0 && date_compare(*d, to) <= 0;
    }
    return found;
}

void apply_filter(const User *user, void *data) {
    Filter *filter = data;
    int matches = 0;

    switch (filter->criteria) {
    case CRITERIA_BALANCE_ABOVE:
        matches = user->balance > filter->amount;
        break;
    case CRITERIA_OPERATIONS_IN_PERIOD:
        matches = has_operation_in_period(user, filter->from, filter->to);
        break;
    case CRITERIA_MORE_OPERATIONS:
        matches = operation_list_count(user->operations) > filter->min_count;
        break;
    }
    if (matches) {
        print_user(user);
        filter->matches++;
    }
}

void show_menu(void) {
    puts("\n=== Billetera virtual ===");
    puts("1. Registrar usuario");
    puts("2. Buscar usuario por id (datos e historial)");
    puts("3. Eliminar usuario");
    puts("4. Depositar dinero");
    puts("5. Retirar dinero");
    puts("6. Consultar saldo");
    puts("7. Listar usuarios por criterio");
    puts("8. Listar todos los usuarios");
    puts("0. Salir");
}

void cmd_register(Wallet *wallet) {
    char name[LINE_SIZE];
    int id;

    printf("Nombre: ");
    read_line(name);
    if (name[0] == '\0') {
        printf("El nombre no puede estar vacío.\n");
    } else {
        wallet_register(wallet, name, &id);
        printf("Usuario registrado con id %d.\n", id);
    }
}

void cmd_find(const Wallet *wallet) {
    int id;
    const User *user;

    if (!read_id(&id)) {
        printf("Id inválido.\n");
    } else {
        user = wallet_find(wallet, id);
        if (!user) {
            show_result(RESULT_NOT_FOUND);
        } else {
            print_user(user);
            print_history(user);
        }
    }
}

void cmd_delete(Wallet *wallet) {
    int id;

    if (!read_id(&id)) {
        printf("Id inválido.\n");
    } else {
        show_result(wallet_remove(wallet, id));
    }
}

void cmd_movement(Wallet *wallet, int is_deposit) {
    int id;
    long long amount;
    Date date;
    Result result;

    if (!read_id(&id)) {
        printf("Id inválido.\n");
    } else if (!read_amount("Monto: ", &amount)) {
        printf("Monto inválido.\n");
    } else if (!read_date("Fecha", &date)) {
        printf("Fecha inválida.\n");
    } else {
        result = is_deposit ? wallet_deposit(wallet, id, amount, date)
                            : wallet_withdraw(wallet, id, amount, date);
        show_result(result);
    }
}

void cmd_balance(const Wallet *wallet) {
    int id;
    const User *user;

    if (!read_id(&id)) {
        printf("Id inválido.\n");
    } else {
        user = wallet_find(wallet, id);
        if (!user) {
            show_result(RESULT_NOT_FOUND);
        } else {
            printf("Saldo de %s: ", user->name);
            print_amount(user->balance);
            putchar('\n');
        }
    }
}

void cmd_list_by_criteria(const Wallet *wallet) {
    Filter filter = {0};
    long value;
    int valid = 1;

    puts("Criterios:");
    puts("  1. Saldo mayor que un monto");
    puts("  2. Con operaciones en un período");
    puts("  3. Con más de N operaciones");
    if (!read_long("Elija un criterio: ", &value)) {
        valid = 0;
        printf("Criterio inválido.\n");
    }
    if (valid) {
        filter.criteria = (int)value;
        switch (filter.criteria) {
        case CRITERIA_BALANCE_ABOVE:
            valid = read_amount("Monto: ", &filter.amount);
            if (!valid) {
                printf("Monto inválido.\n");
            }
            break;
        case CRITERIA_OPERATIONS_IN_PERIOD:
            valid = read_date("Desde", &filter.from) && read_date("Hasta", &filter.to) &&
                    date_compare(filter.from, filter.to) <= 0;
            if (!valid) {
                printf("Período inválido.\n");
            }
            break;
        case CRITERIA_MORE_OPERATIONS:
            valid = read_long("Cantidad de operaciones (N): ", &value) && value >= 0;
            if (valid) {
                filter.min_count = (size_t)value;
            } else {
                printf("Número inválido.\n");
            }
            break;
        default:
            valid = 0;
            printf("Criterio inválido.\n");
        }
    }
    if (valid) {
        wallet_for_each(wallet, apply_filter, &filter);
        printf("Usuarios encontrados: %zu\n", filter.matches);
    }
}

void cmd_list_all(const Wallet *wallet) {
    if (wallet_count(wallet) == 0) {
        puts("No hay usuarios registrados.");
    } else {
        wallet_for_each(wallet, print_user_action, NULL);
        printf("Total: %zu usuario(s).\n", wallet_count(wallet));
    }
}

int main(void) {
    Wallet *wallet = wallet_create();
    long option = -1;

    while (option != 0) {
        show_menu();
        if (!read_long("Opción: ", &option)) {
            printf("Opción inválida.\n");
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
            case 0:
                wallet_destroy(wallet);
                puts("¡Hasta luego!");
                break;
            default:
                printf("Opción inválida.\n");
            }
        }
    }
    return EXIT_SUCCESS;
}
