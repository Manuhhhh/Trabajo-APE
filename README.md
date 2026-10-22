# Billetera Virtual

Proyecto del curso PCII 2026. Simulación en C de una billetera virtual: se registran usuarios, cada uno con un saldo y un historial de ingresos y egresos, que se pueden consultar y listar según distintos criterios. No realiza transacciones reales.

El análisis, la comparación de estructuras y la especificación de los TAD están en [`Analisis y diseño - Billetera Virtual.pdf`](Analisis%20y%20dise%C3%B1o%20-%20Billetera%20Virtual.pdf).

## Compilar y ejecutar

Requiere `gcc` y `make`.

```sh
make        # compila en build/wallet
make run    # compila y ejecuta
make clean  # borra build/
```

Se compila con `-std=c11 -Wall -Wextra -Wpedantic`, sin warnings.

## Uso

El programa muestra un menú numerado y se interactúa escribiendo la opción:

| Opción | Acción |
| --- | --- |
| 1 | Registrar usuario (el id lo asigna el sistema) |
| 2 | Buscar usuario por id (datos e historial) |
| 3 | Eliminar usuario (solo con saldo cero; se elimina también su historial) |
| 4 | Depositar dinero |
| 5 | Retirar dinero (verifica saldo suficiente) |
| 6 | Consultar saldo |
| 7 | Listar usuarios por criterio: saldo mayor que un monto, con operaciones en un período, o con más de N operaciones |
| 8 | Listar todos los usuarios (en orden de id) |
| 0 | Salir |

- Los montos se ingresan con punto decimal, por ejemplo `100.50`.
- Las fechas se ingresan como `dd/mm/aaaa`; con Enter se usa la fecha de hoy. Se aceptan años entre 1900 y 2100.
- Si un dato no tiene el formato esperado, se muestra `Entrada incorrecta, intente de nuevo.` y se vuelve al menú.
- Cada depósito o retiro registra la operación en el historial y actualiza el saldo en un único paso.

## Estructura del proyecto

| Archivo | Contenido |
| --- | --- |
| `src/main.c` | Menú, lectura de datos y listados. Es la única parte que interactúa con el usuario |
| `src/wallet.c`, `include/wallet.h` | TAD `Wallet`: registro de usuarios y reglas de depósito, retiro y baja. Asigna los ids |
| `src/user_tree.c`, `include/user_tree.h` | TAD `UserTree`: AVL de usuarios indexado por id |
| `src/user.c`, `include/user.h` | TAD `User`: id, nombre, saldo e historial |
| `src/operation_list.c`, `include/operation_list.h` | TAD `OperationList`: historial de operaciones (arreglo dinámico) |
| `src/operation.c`, `include/operation.h` | TAD `Operation`: un depósito o retiro registrado |
| `src/date.c`, `include/date.h` | TAD `Date`: validación, comparación y fecha de hoy |

Cada TAD tiene su par `.h`/`.c` y un único `struct`, que se declara solo en el `.c`. Dependencias, de arriba hacia abajo: `main` → `wallet` → `user_tree` → `user` → `operation_list` → `operation` → `date`. La lógica no hace ninguna entrada ni salida; todo lo que se lee o imprime está en `main.c`.

## Estructuras de datos

- **Usuarios:** árbol AVL ordenado por id. Búsqueda, alta y baja en O(log n) en el peor caso; el recorrido in-order entrega los usuarios ordenados por id.
- **Historial de operaciones:** arreglo dinámico de punteros a `Operation` por usuario. Agregar al final en O(1) amortizado y acceso por posición en O(1).
- **Montos:** en centavos, como `long long`.

## Decisiones de diseño

### AVL para los usuarios

- **Por qué un árbol y no un arreglo o una lista.** Todas las consultas son por id (buscar, depositar, retirar, eliminar). En un arreglo ordenado la búsqueda es O(log n), pero cada alta o baja desplaza elementos (O(n)); en una lista todo es O(n). Un árbol de búsqueda balanceado da O(log n) en las tres operaciones y su recorrido in-order entrega los usuarios ya ordenados por id, que es lo que necesitan los listados.
- **Por qué AVL y no un ABB simple.** Los ids los asigna el sistema de forma creciente (`next_user_id`), así que las altas llegan siempre ordenadas. En un ABB sin balanceo eso lo degenera en una lista enlazada (O(n) por operación). El AVL se rebalancea con rotaciones y mantiene la altura en O(log n).
- **Los ids no se reutilizan.** `next_user_id` solo crece, aunque se elimine un usuario, así un id nunca apunta a dos personas distintas en momentos distintos. El contador de ids de operaciones (`next_operation_id`) es global a la billetera por la misma razón.
- **Se guarda la altura, no el factor de balance.** Cada nodo almacena solo `height`; el balance (`altura izq - altura der`) se calcula cuando hace falta. Guardar ambos obligaba a mantenerlos sincronizados en cada rotación.
- **Un único `rebalance` para alta y baja.** Después de insertar o eliminar, la recursión vuelve hacia la raíz llamando a `rebalance` en cada nodo del camino. Los casos izquierda-derecha y derecha-izquierda se detectan comparando las alturas de los nietos con `<` y `>` estrictos; con alturas iguales (situación que solo ocurre al eliminar) corresponde una rotación simple, y la comparación estricta la elige correctamente.
- **Baja con dos hijos.** `remove_min` desconecta el sucesor in-order (el menor del subárbol derecho) y el sucesor ocupa el lugar del nodo eliminado **reenlazando punteros**, en lugar de copiar el usuario al nodo a borrar. `user_tree_remove` libera el nodo y devuelve el `User` intacto en `*removed`, para que `wallet_remove` lo destruya junto con su historial.
- **El árbol es su propio nodo.** `UserTree` es a la vez el nodo y el subárbol que cuelga de él (el árbol vacío es `NULL`), así el TAD tiene un único `struct` y no hace falta un tipo `Node` aparte.
- **Recursión.** Las operaciones del árbol son recursivas porque el AVL garantiza profundidad O(log n): no hay riesgo de desbordar la pila y el código queda mucho más corto que con versiones iterativas.

### Historial de operaciones

- **Arreglo dinámico con duplicación de capacidad.** Se parte de `INITIAL_CAPACITY` (4) y la capacidad se duplica al llenarse. Agregar al final es O(1) amortizado y el acceso por posición es O(1), que es lo que usan el historial y el filtro por período al recorrer. No se necesitan inserciones en el medio, que es donde una lista enlazada tendría ventaja; además el arreglo usa menos memoria por operación y recorre memoria contigua.
- **Orden de registro, no de fecha.** La fecha la ingresa el usuario, por lo que pueden cargarse movimientos con fechas desordenadas. El historial se muestra en el orden en que se registraron, y el filtro por período recorre todo el historial (O(k) por usuario) en lugar de asumir que está ordenado por fecha.

### Montos y saldos

- **Centavos en `long long`.** Todo el modelo (el monto de `Operation`, el saldo de `User`) usa enteros en centavos: `float`/`double` acumulan errores de redondeo en sumas y restas repetidas, y comparar saldos sería poco fiable. `long long` evita desbordes con montos grandes.
- **La conversión a centavos ocurre solo en la entrada.** `read_amount` lee un `double` y lo pasa a centavos con `(long long)(value * 100)`; a partir de ahí no vuelven a aparecer números de coma flotante. Esa conversión **trunca** y puede perder un centavo con algunos decimales (por ejemplo `0.29` se guarda como 28 centavos, porque `0.29 * 100` da `28.999...`). Es una limitación aceptada del parseo con `scanf("%lf")`.

### Lógica separada de la interfaz

- **Ningún `struct` en los headers.** Los seis TAD (`Date`, `Operation`, `OperationList`, `User`, `UserTree`, `Wallet`) son tipos opacos: el header solo tiene `typedef struct X X;` y las funciones; los campos viven en el `.c`. Quien use un TAD solo accede a través de sus funciones (`user_balance`, `operation_date`, …) y no puede dejarlo en un estado inconsistente.
- **Un objeto por TAD.** Cada módulo define un único `struct`. Por eso la billetera quedó separada de las operaciones: `Wallet` solo conoce usuarios y reglas, `User` solo su historial y saldo, y `OperationList` y `Operation` no saben nada de billeteras.
- **Quién es dueño de qué.** Cada contenedor libera lo que guarda: `Wallet` → `UserTree` → `User` → `OperationList` → `Operation` → `Date`. `operation_create` guarda su propia copia de la fecha, así que quien le pasa una `Date` sigue siendo dueño de la suya (`main.c` la libera con `date_destroy`).
- **Solo lectura hacia afuera.** `wallet_find`, `user_operations`, `operation_list_get` y `operation_date` devuelven `const`, de modo que la interfaz puede consultar pero no modificar saldos saltándose las validaciones.
- **El TAD devuelve `Result`, no imprime.** `wallet.c` no hace `printf`: devuelve un código (`RESULT_NOT_FOUND`, `RESULT_INSUFFICIENT_FUNDS`, …) y `main.c` decide el mensaje (`show_result`).
- **Recorrido con arreglo.** `wallet_users(wallet)` devuelve un arreglo nuevo de `const User *` en orden de id (un recorrido in-order, O(n)), que el llamador libera con `free`. Los listados y filtros viven en `main.c` como `for` comunes, sin exponer los nodos del árbol y sin punteros a función ni `void *`. El costo es una copia de punteros por listado (no de usuarios), y los usuarios siguen siendo de la billetera.

### Movimientos

- **Un único paso para registrar y actualizar.** `wallet.c` valida y, solo si todo es correcto, llama a `user_record`, que agrega la operación **y** actualiza el saldo. No existe un estado intermedio donde historial y saldo no coincidan. Depósito y retiro son envoltorios de la misma función porque solo difieren en el signo y en la verificación de saldo.
- **Orden de las validaciones.** Se verifica monto positivo, existencia del usuario y, en retiros, saldo suficiente, de modo que el mensaje corresponde al primer error real. La fecha ya no se valida acá: una `Date` solo se puede crear si es válida (`date_create` devuelve `NULL` si no), por lo que `RESULT_INVALID_DATE` dejó de existir.
- **Eliminar solo con saldo cero.** Evita que desaparezca dinero de la billetera sin un retiro registrado. Al eliminar un usuario se destruye también su historial.

### Fechas

- **Siempre válida.** `date_create` rechaza las fechas inexistentes, así que toda `Date` que circula por el programa es válida y los demás módulos no necesitan revalidarla.
- **Rango 1900-2100 y calendario gregoriano.** `days_in_month` considera los años bisiestos (divisibles por 4, salvo los múltiplos de 100 que no lo son de 400). El rango acotado es suficiente para el dominio y permite rechazar fechas absurdas.
- **Comparación con clave `aaaammdd`.** `key` convierte la fecha en `año*10000 + mes*100 + día`, que conserva el orden cronológico: comparar fechas es comparar dos enteros, más simple y menos propenso a errores que comparar año, luego mes y luego día.
- **No se valida que la fecha no sea futura.** Se permite cargar movimientos con cualquier fecha válida del rango.

### Lectura de datos

- **`scanf` y `clear_buffer`.** Cada lectura usa `scanf` con el formato que corresponde (`%d`, `%lf`, `%s`, `%d/%d/%d`) y se verifica que devuelva la cantidad de campos esperada. `scanf` deja en el buffer el salto de línea (o el resto de una entrada inválida como `abc`) y la siguiente lectura fallaría sin avanzar; por eso cada función de lectura llama a `clear_buffer`, que descarta lo que quedó hasta el salto de línea.
- **Una función, una tarea completa.** `read_int`, `read_amount`, `read_date` y `read_string` muestran el mensaje, leen, limpian el buffer, avisan si la entrada es incorrecta y devuelven 1 o 0. Así `main.c` encadena las lecturas con `&&` (`read_int(...) && read_amount(...) && read_date(...)`) y el flujo se corta en el primer dato inválido, sin repetir mensajes en cada comando.
- **`Enter = hoy` en la fecha.** `scanf` no puede distinguir una línea vacía, por eso `read_date` mira el primer carácter con `getchar`: si es el salto de línea usa `date_today`, y si no lo devuelve al buffer con `ungetc` para que `scanf` lo lea.

### Convenciones de código

- Cada función tiene una única salida (`return` al final, con una variable `result` cuando hace falta) y no se usan operadores ternarios.
- Las comparaciones con punteros son explícitas (`!= NULL`, `== NULL`).
- Los comentarios de cada función están en un prototipo al inicio del archivo; los de los headers públicos describen el contrato. Las funciones auxiliares internas de cada módulo son `static`.

## Limitaciones conocidas

- Las lecturas son simples a propósito: `scanf("%s")` no limita el largo (el nombre se lee como una sola palabra y `wallet_register` lo trunca a 49 caracteres), y `12abc` se lee como 12.
- Si la entrada termina (EOF) sin haber elegido la opción 0, `clear_buffer` y el menú quedan en un bucle infinito. Al usar el programa por pipe hay que terminar siempre con `0`.
- No se comprueba el resultado de `malloc`/`realloc`.
- Los datos viven solo en memoria: al cerrar el programa se pierden.
