# Cifrado de rueda de Alberti

Programa en C que implementa una rueda externa fija y una rueda interna configurable mediante una clave. Permite cifrar y descifrar archivos de forma independiente.

## Requisitos

- GCC o un compilador compatible con C11.
- Un archivo de texto como entrada.
- Una clave formada por las 26 letras mayúsculas de `A` a `Z`, sin repetir.

## Compilación

Desde la carpeta `cifradoAlberti/source`:

```bash
gcc -Wall -Wextra -Werror -std=c11 cifradoAlberti.c -o cifradoAlberti
```

También se puede compilar desde la raíz del proyecto:

```bash
gcc -Wall -Wextra -Werror -std=c11 \
  cifradoAlberti/source/cifradoAlberti.c \
  -o cifradoAlberti/source/cifradoAlberti
```

## Sintaxis

```text
./cifradoAlberti <c|d> <entrada> <salida> <posicion> <periodo> <+|-> <clave>
```

Los argumentos son:

| Argumento | Descripción |
|---|---|
| `c` | Cifra el archivo de entrada. |
| `d` | Descifra el archivo de entrada. |
| `entrada` | Archivo que se leerá. |
| `salida` | Archivo nuevo donde se guardará el resultado. Debe ser diferente de la entrada. |
| `posicion` | Posición inicial de la rueda interna, entre `0` y `25`. |
| `periodo` | Número de letras procesadas antes de girar la rueda. Debe ser mayor que `0`. |
| `+` | Gira la rueda una posición hacia adelante después de cada período. |
| `-` | Gira la rueda una posición hacia atrás después de cada período. |
| `clave` | Permutación de las 26 letras mayúsculas de `A-Z`. |

Los espacios, números, signos de puntuación y saltos de línea se copian sin cambios. Solo se transforman las letras ASCII `A-Z` y `a-z`.

## Ejemplo completo

Los siguientes comandos deben ejecutarse desde `cifradoAlberti/source`.

### 1. Crear el archivo de entrada

```bash
printf 'Mensaje secreto de Alberti.\nSegunda linea.\n' > mensaje_original.txt
```

Archivo generado: `mensaje_original.txt`.

### 2. Cifrar

```bash
./cifradoAlberti c \
  mensaje_original.txt \
  mensaje_cifrado.txt \
  3 \
  4 \
  - \
  ZYXWVUTSRQPONMLKJIHGFEDCBA
```

El resultado se guarda en `mensaje_cifrado.txt`.

En este ejemplo:

- La rueda comienza en la posición `3`.
- Gira una posición hacia atrás cada `4` letras.
- La rueda interna usa el alfabeto invertido.

### 3. Descifrar

```bash
./cifradoAlberti d \
  mensaje_cifrado.txt \
  mensaje_recuperado.txt \
  3 \
  4 \
  - \
  ZYXWVUTSRQPONMLKJIHGFEDCBA
```

El resultado se guarda en `mensaje_recuperado.txt`. Debe coincidir con `mensaje_original.txt`.

Se puede comprobar con:

```bash
cmp mensaje_original.txt mensaje_recuperado.txt
```

Si no se muestra ningún mensaje, ambos archivos son idénticos.

## Otras claves válidas

Alfabeto normal, equivalente a desplazamientos simples entre las ruedas:

```text
ABCDEFGHIJKLMNOPQRSTUVWXYZ
```

Alfabeto invertido:

```text
ZYXWVUTSRQPONMLKJIHGFEDCBA
```

Alfabeto mezclado:

```text
QWERTYUIOPASDFGHJKLZXCVBNM
```

La misma combinación de `posicion`, `periodo`, dirección y `clave` debe utilizarse para descifrar que la utilizada durante el cifrado.

## Errores frecuentes

- La clave debe contener exactamente 26 caracteres.
- La clave solo puede usar letras mayúsculas ASCII de `A` a `Z`.
- No se permiten letras repetidas en la clave.
- La posición debe estar entre `0` y `25`.
- El período debe ser un número entero mayor que `0`.
- El archivo de entrada y el archivo de salida deben tener nombres diferentes.
- Para indicar la dirección `+` en algunos shells puede ser necesario escribirlo entre comillas: `"+"`.
