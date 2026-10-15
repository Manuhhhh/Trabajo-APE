# Billetera Virtual

Proyecto del curso PCII 2026. Simulación en C de una billetera virtual: se registran usuarios, cada uno con un saldo y un historial de ingresos y egresos, que se pueden consultar y listar según distintos criterios. No realiza transacciones reales.

## Estructuras de datos

- **Usuarios:** árbol AVL ordenado por id. Búsqueda, alta y baja en O(log n) en el peor caso; el recorrido in-order entrega los usuarios ordenados por id.
- **Historial de operaciones:** arreglo dinámico por usuario. Agregar al final en O(1) amortizado y acceso por posición en O(1).
- Los montos se guardan en **centavos** (`long long`) para evitar errores de redondeo.

El detalle del análisis, la comparación de estructuras y la especificación de los TAD está en [`Analisis y diseño - Billetera Virtual.pdf`](Analisis%20y%20dise%C3%B1o%20-%20Billetera%20Virtual.pdf).

## Estructura del proyecto

| Archivo | Contenido |
| --- | --- |
| `src/main.c` | Menú e interacción con el usuario |
| `src/wallet.c`, `include/wallet.h` | TAD `Wallet` (AVL de usuarios) |
| `src/operation_list.c`, `include/operation_list.h` | TAD `OperationList` (historial) |
| `include/operation.h`, `include/user.h` | Tipos `Operation` y `User` |
| `src/date.c`, `include/date.h` | Validación y comparación de fechas |

## Compilar y ejecutar

Requiere `gcc` y `make`.

```sh
make        # compila en build/wallet
make run    # compila y ejecuta
make clean  # borra build/
```

## Funcionalidades

1. Registrar usuario (el id lo asigna el sistema)
2. Buscar usuario por id (datos e historial)
3. Eliminar usuario (solo con saldo cero; se elimina también su historial)
4. Depositar dinero
5. Retirar dinero (verifica saldo suficiente)
6. Consultar saldo
7. Listar usuarios por criterio: saldo mayor que un monto, con operaciones en un período, o con más de N operaciones
8. Listar todos los usuarios

Cada ingreso o egreso registra la operación en el historial y actualiza el saldo en un único paso. Las fechas se ingresan como `dd/mm/aaaa` (Enter usa la fecha de hoy) y los montos con punto decimal, por ejemplo `100.50`.
