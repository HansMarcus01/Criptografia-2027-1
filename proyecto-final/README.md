# Implementación educativa de DES en C

Este proyecto implementa el algoritmo de cifrado por bloques DES (Data
Encryption Standard) y ofrece una interfaz de consola para cifrar y descifrar
archivos. El código está separado en un módulo que implementa el bloque DES,
otro que genera y lee llaves, y un programa principal que administra los
archivos.

Esta guía explica los conceptos necesarios para seguir el código, la
responsabilidad de cada archivo y el propósito de cada función y parámetro.

> **Advertencia:** DES está obsoleto. Su llave efectiva tiene solo 56 bits y
> puede encontrarse mediante búsqueda exhaustiva. El modo CBC utilizado aquí
> tampoco autentica los datos. Esta implementación es para aprendizaje y no
> debe usarse para proteger información real.

## 1. Ideas básicas

### ¿Qué hace un cifrado de bloque?

Un cifrado de bloque transforma una cantidad fija de datos mediante una llave.
DES trabaja con bloques de **8 bytes (64 bits)**. Si el archivo no tiene una
longitud múltiplo de 8 bytes, el programa añade relleno para completar el
último bloque. Incluso un archivo vacío recibe un bloque de relleno.

El algoritmo de bloque no recibe directamente un archivo completo: el programa
parte el archivo en bloques y aplica DES a cada uno.

### Llave DES

DES recibe 8 bytes de llave. Cada byte contiene 7 bits efectivos y 1 bit de
paridad, por lo que la llave tiene **56 bits efectivos**. El algoritmo no
genera esa llave: el módulo `generador_llaves.c` obtiene bytes aleatorios del
sistema operativo, ajusta la paridad impar y los conserva en un archivo aparte.

La llave es necesaria para descifrar. Por eso:

- La llave **no** se agrega al archivo cifrado.
- El archivo de llave debe mantenerse privado y respaldarse si se necesita
  descifrar posteriormente.
- Al descifrar se debe indicar el mismo archivo de llave que se generó al
  cifrar; no se genera una llave nueva.
- El programa no reemplaza una llave existente con el mismo nombre.

### Feistel y rondas

DES primero permuta los 64 bits del bloque, y después los separa en dos mitades
de 32 bits: izquierda (`L`) y derecha (`R`). En cada una de sus 16 rondas,
calcula una función `F` sobre la mitad derecha y una subllave:

```text
L_nueva = R_anterior
R_nueva = L_anterior XOR F(R_anterior, subllave)
```

`XOR` es una operación bit a bit que produce 1 cuando los bits comparados son
diferentes. Después de la ronda 16, DES intercambia las mitades, aplica la
permutación final y obtiene los 64 bits cifrados.

La función `F` expande 32 bits a 48, los combina con la subllave mediante XOR,
los procesa con ocho cajas de sustitución (`S-boxes`) y aplica una permutación.
La planificación de llave transforma la llave inicial en 16 subllaves, una por
ronda.

Para descifrar se ejecutan las mismas rondas Feistel en orden inverso de
subllaves.

### Modo CBC y formato del archivo cifrado

El programa usa CBC (Cipher Block Chaining). El primer bloque se combina con un
vector de inicialización (IV) aleatorio; cada bloque siguiente se combina con
el bloque cifrado anterior:

```text
C1 = DES(P1 XOR IV)
Cn = DES(Pn XOR C(n-1))
```

Al descifrar, el resultado DES de cada bloque se combina con el IV o con el
bloque cifrado anterior para recuperar el bloque original.

Los archivos de entrada y salida se tratan como secuencias de bytes. El
cifrado se guarda como texto hexadecimal con este formato:

```text
DES-CBC-1
IV=<16 dígitos hexadecimales>
<texto cifrado en hexadecimal>
```

La línea de texto cifrado tiene dos dígitos hexadecimales por byte y termina
con un salto de línea. El IV se guarda para que el descifrado pueda comenzar;
no es una llave y no tiene que mantenerse secreto.

El relleno es PKCS#7: si hacen falta `n` bytes para completar un bloque, se
añaden `n` bytes cuyo valor también es `n`. Si el texto ya tiene una longitud
múltiplo de 8, se agrega un bloque completo con ocho bytes de valor `8`.

## 2. Archivos del proyecto

| Archivo | Responsabilidad |
|---|---|
| `source/des.h` | Declara el tamaño del bloque y las operaciones públicas de DES. |
| `source/des.c` | Implementa las tablas, la planificación de subllaves y las rondas DES. |
| `source/generador_llaves.h` | Declara la interfaz para aleatoriedad y archivos de llave. |
| `source/generador_llaves.c` | Obtiene bytes del sistema operativo y crea/lee archivos de llave. |
| `source/main.c` | Valida argumentos, lee y escribe archivos, y aplica CBC con relleno. |
| `tests/test_des.c` | Comprueba DES con un vector de prueba conocido y verifica el descifrado. |

No se declara ningún `struct`, `enum` ni `typedef` propio. Los tipos principales
son tipos estándar de C: `uint8_t` representa un byte sin signo, `uint32_t` un
entero sin signo de 32 bits, `uint64_t` uno de 64 bits, `size_t` tamaños e
índices, `FILE *` un archivo de la biblioteca estándar y `int` estados de
retorno.

## 3. Compilación y ejecución

Se requiere GCC o un compilador C11 compatible y un sistema tipo Unix que
proporcione `/dev/urandom`.

Desde la carpeta `proyecto-final/source`:

```bash
gcc -Wall -Wextra -Werror -std=c11 main.c des.c generador_llaves.c -o des
```

O desde la raíz del repositorio:

```bash
gcc -Wall -Wextra -Werror -std=c11 \
  proyecto-final/source/main.c \
  proyecto-final/source/des.c \
  proyecto-final/source/generador_llaves.c \
  -o proyecto-final/source/des
```

El programa carga cada archivo completo en memoria; por ello, el tamaño
disponible de memoria limita el tamaño de archivo procesable.

### Cifrar

```text
./des c <entrada> <salida>
```

`c` selecciona el cifrado. Por ejemplo, si el archivo de entrada se llama
`mensaje.txt`, el programa crea `llavesmensaje.txt.env` **en la misma carpeta
que `mensaje.txt`**:

```bash
printf 'Mensaje secreto.\n' > mensaje.txt
./des c mensaje.txt mensaje.cif
```

El archivo de llave contiene una línea como `DES_KEY=<16 dígitos hex>` y se
crea solicitando permisos `0600` (lectura y escritura del propietario). Si ya
existe un archivo con ese nombre, el cifrado falla en lugar de reemplazarlo.
Esto evita perder una llave que pudiera corresponder a un cifrado anterior.

La salida debe tener una ruta distinta de la entrada y no debe sobrescribir el
archivo de llave. Si la ruta de salida ya existe, se abre para escritura y su
contenido anterior se reemplaza.

### Descifrar

```text
./des d <entrada_cifrada> <salida> <archivo_llave>
```

`d` selecciona el descifrado. Se debe proporcionar la ruta del archivo de llave
creado al cifrar:

```bash
./des d mensaje.cif recuperado.txt llavesmensaje.txt.env
cmp mensaje.txt recuperado.txt
```

La salida debe ser distinta del archivo cifrado y del archivo de llave. Si la
ruta de salida ya existe, se reemplaza su contenido.

## 4. Tipos y constantes públicos

### `source/des.h`

#### `DES_TAM_BLOQUE`

Macro con valor `8`. Representa el tamaño en bytes de un bloque DES y también
el tamaño de la llave que recibe la interfaz pública.

#### `des_cifrar_bloque`

```c
void des_cifrar_bloque(const uint8_t entrada[DES_TAM_BLOQUE],
                       uint8_t salida[DES_TAM_BLOQUE],
                       const uint8_t llave[DES_TAM_BLOQUE]);
```

Cifra exactamente un bloque de 8 bytes. No añade relleno ni aplica CBC: esas
responsabilidades pertenecen al programa principal.

| Parámetro | Significado |
|---|---|
| `entrada` | Los 8 bytes originales que se van a cifrar. |
| `salida` | Arreglo donde se escriben los 8 bytes cifrados. |
| `llave` | La llave DES de 8 bytes. Los bits de paridad no se usan en las rondas. |

La función no devuelve un estado porque sus arreglos tienen tamaño fijo y no
realiza operaciones de entrada/salida.

#### `des_descifrar_bloque`

```c
void des_descifrar_bloque(const uint8_t entrada[DES_TAM_BLOQUE],
                          uint8_t salida[DES_TAM_BLOQUE],
                          const uint8_t llave[DES_TAM_BLOQUE]);
```

Descifra un bloque de 8 bytes cifrado con DES y la llave indicada.

| Parámetro | Significado |
|---|---|
| `entrada` | Los 8 bytes cifrados. |
| `salida` | Arreglo donde se escriben los 8 bytes descifrados. |
| `llave` | La misma llave de 8 bytes usada para cifrar. |

## 5. Implementación DES: `source/des.c`

### Tablas constantes

Las tablas `static const uint8_t` contienen las permutaciones definidas por
DES. `static` limita su uso a este archivo y `const` indica que no se modifican.
Los números de las tablas identifican posiciones de bits comenzando por 1,
según el orden definido por el estándar DES.

| Constante | Propósito |
|---|---|
| `permutacion_inicial` | Reordena los 64 bits al comienzo del cifrado/descifrado. |
| `permutacion_final` | Reordena los bits al final; es la inversa de la permutación inicial. |
| `expansion` | Expande una mitad de 32 bits a 48 bits para combinarla con una subllave. |
| `permutacion_p` | Reordena los 32 bits que salen de las cajas S. |
| `seleccion_llave_1` | Selecciona 56 bits de la llave de 64 bits, omitiendo los bits de paridad. |
| `seleccion_llave_2` | Selecciona y permuta 48 bits para formar una subllave de ronda. |
| `desplazamientos` | Cantidad de rotaciones circulares de las mitades de llave en cada una de las 16 rondas. |
| `cajas_s` | Las ocho cajas de sustitución de DES. Cada una transforma 6 bits en 4 bits. |

### Funciones internas

Son funciones `static`: solo pueden usarse desde `des.c`.

#### `leer_u64`

```c
static uint64_t leer_u64(const uint8_t bytes[8]);
```

Interpreta ocho bytes como un entero de 64 bits, colocando primero el byte más
significativo. Este orden explícito hace que el resultado no dependa del orden
de bytes nativo del procesador.

| Parámetro | Significado |
|---|---|
| `bytes` | Los ocho bytes que se combinan en un entero. |

#### `escribir_u64`

```c
static void escribir_u64(uint64_t valor, uint8_t bytes[8]);
```

La operación inversa de `leer_u64`: separa un entero de 64 bits en ocho bytes,
del más significativo al menos significativo.

| Parámetro | Significado |
|---|---|
| `valor` | El entero de 64 bits que se va a separar. |
| `bytes` | Arreglo de salida de ocho bytes. |

#### `permutar`

```c
static uint64_t permutar(uint64_t entrada, const uint8_t *tabla,
                         size_t cantidad, unsigned int bits_entrada);
```

Aplica una tabla de permutación genérica. Para cada posición en `tabla`, copia
el bit indicado de `entrada` al siguiente bit de salida.

| Parámetro | Significado |
|---|---|
| `entrada` | Valor cuyos bits se van a reordenar. |
| `tabla` | Arreglo con las posiciones de bits, numeradas desde 1. |
| `cantidad` | Número de posiciones de la tabla y número de bits de salida. |
| `bits_entrada` | Ancho del valor de entrada; permite convertir la posición de tabla en un desplazamiento de bits. |

El resultado es `uint64_t` para poder representar salidas de hasta 64 bits.

#### `funcion_f`

```c
static uint32_t funcion_f(uint32_t mitad, uint64_t subllave);
```

Calcula la función `F` de una ronda DES: expande la mitad derecha a 48 bits,
aplica XOR con la subllave, separa el resultado en ocho grupos de 6 bits,
consulta una caja S por grupo y permuta los 32 bits resultantes.

| Parámetro | Significado |
|---|---|
| `mitad` | Mitad de bloque de 32 bits sobre la que opera la ronda. |
| `subllave` | Subllave de ronda de 48 bits guardada en un entero de 64 bits. |

Devuelve el resultado de 32 bits de la función `F`.

#### `crear_subllaves`

```c
static void crear_subllaves(const uint8_t llave[8], uint64_t subllaves[16]);
```

Genera las 16 subllaves DES. Primero aplica `seleccion_llave_1`, divide los
56 bits en dos mitades de 28 bits, las rota según `desplazamientos` y aplica
`seleccion_llave_2` para guardar 48 bits por ronda.

| Parámetro | Significado |
|---|---|
| `llave` | Llave original de 8 bytes. |
| `subllaves` | Arreglo de salida con 16 enteros; cada uno contiene una subllave de 48 bits. |

#### `transformar_bloque`

```c
static void transformar_bloque(const uint8_t entrada[8], uint8_t salida[8],
                               const uint8_t llave[8], int descifrar);
```

Implementa el flujo común de cifrado y descifrado de un bloque. Convierte el
bloque en un entero, aplica la permutación inicial, ejecuta las 16 rondas
Feistel, intercambia las mitades y aplica la permutación final. Para descifrar,
usa las subllaves desde la ronda 16 hasta la ronda 1.

| Parámetro | Significado |
|---|---|
| `entrada` | Bloque de entrada de 8 bytes. |
| `salida` | Arreglo que recibe el bloque transformado de 8 bytes. |
| `llave` | Llave DES de 8 bytes. |
| `descifrar` | `0` selecciona subllaves en orden de cifrado; cualquier valor distinto de cero las recorre en orden inverso. |

## 6. Generación y lectura de llaves

### `source/generador_llaves.h`

#### `LLAVE_DES_TAM`

Macro con valor `8`. Indica la cantidad de bytes que ocupa una llave DES.

#### `obtener_bytes_aleatorios`

```c
int obtener_bytes_aleatorios(uint8_t *destino, size_t cantidad);
```

Lee exactamente `cantidad` bytes de `/dev/urandom`. Repite lecturas parciales
y vuelve a intentar si la llamada al sistema se interrumpe.

| Parámetro | Significado |
|---|---|
| `destino` | Memoria donde se depositan los bytes aleatorios; debe tener capacidad suficiente. |
| `cantidad` | Cantidad de bytes que se solicitan. |

Devuelve `0` si tuvo éxito y `-1` si falló. En caso de error conserva el código
en `errno`; el llamador puede mostrarlo con `perror`.

#### `generar_archivo_llave`

```c
int generar_archivo_llave(const char *archivo_entrada,
                          uint8_t llave[LLAVE_DES_TAM],
                          char *ruta_llave, size_t capacidad_ruta);
```

Genera una llave DES aleatoria, ajusta sus bits de paridad y crea su archivo
junto al archivo de entrada. Por ejemplo, `datos.txt` produce
`llavesdatos.txt.env`.

El archivo contiene `DES_KEY=` seguido de los 16 dígitos hexadecimales de la
llave. Se crea con `O_EXCL`, de modo que falla si ya existe y no lo reemplaza.
Solicita permisos `0600`.

| Parámetro | Significado |
|---|---|
| `archivo_entrada` | Ruta del archivo cuyo nombre se usará para derivar el nombre del archivo de llave. |
| `llave` | Arreglo de salida donde se guarda la llave binaria de 8 bytes. |
| `ruta_llave` | Arreglo de salida que recibe la ruta completa generada. |
| `capacidad_ruta` | Capacidad del arreglo `ruta_llave`, en bytes. |

Devuelve `0` al completar la generación y escritura, o `-1` si falla. Los
detalles de error quedan en `errno`.

#### `leer_archivo_llave`

```c
int leer_archivo_llave(const char *ruta_llave,
                       uint8_t llave[LLAVE_DES_TAM]);
```

Lee el formato `DES_KEY=<16 dígitos hex>` y convierte los dígitos a los ocho
bytes que usa DES. Acepta dígitos hexadecimales en mayúsculas o minúsculas, una
nueva línea final opcional y no acepta contenido adicional.

| Parámetro | Significado |
|---|---|
| `ruta_llave` | Ruta al archivo `.env` de llave. |
| `llave` | Arreglo de salida donde se guardan los ocho bytes decodificados. |

Devuelve `0` si la llave pudo leerse y `-1` si el archivo no se pudo abrir,
leer o validar.

### Funciones internas del generador

#### `escribir_todo`

```c
static int escribir_todo(int descriptor, const char *datos, size_t cantidad);
```

Escribe todos los bytes solicitados en un descriptor de archivo, aunque
`write` complete solo una parte. Si `write` es interrumpido, reintenta.

| Parámetro | Significado |
|---|---|
| `descriptor` | Descriptor abierto donde se escriben los datos. |
| `datos` | Bytes que se van a escribir. |
| `cantidad` | Número de bytes a escribir. |

Devuelve `0` al completar la escritura y `-1` si ocurre un error.

#### `ajustar_paridad_impar`

```c
static void ajustar_paridad_impar(uint8_t llave[LLAVE_DES_TAM]);
```

Ajusta el bit menos significativo de cada byte para que los ocho bits del byte
tengan un número impar de bits en `1`. Ese bit es de paridad y no aumenta los
56 bits efectivos de DES.

| Parámetro | Significado |
|---|---|
| `llave` | Llave de 8 bytes que se modifica en el lugar. |

#### `construir_ruta_llave`

```c
static int construir_ruta_llave(const char *archivo_entrada,
                                char *ruta_llave, size_t capacidad_ruta);
```

Construye una ruta en la misma carpeta de `archivo_entrada`, con el prefijo
`llaves` y el sufijo `.env`.

| Parámetro | Significado |
|---|---|
| `archivo_entrada` | Ruta de entrada, de la que se extrae carpeta y nombre base. |
| `ruta_llave` | Arreglo que recibirá la ruta construida. |
| `capacidad_ruta` | Tamaño del arreglo `ruta_llave`. |

Devuelve `0` si la ruta cabe en el arreglo y `-1` si no puede construirla.

#### `valor_hexadecimal`

```c
static int valor_hexadecimal(char caracter);
```

Convierte un carácter hexadecimal (`0`–`9`, `a`–`f` o `A`–`F`) en su valor
numérico entre 0 y 15.

| Parámetro | Significado |
|---|---|
| `caracter` | Carácter que se desea convertir. |

Devuelve el valor numérico o `-1` si el carácter no es hexadecimal.

## 7. Programa principal: `source/main.c`

### Constante `TAM_RUTA`

Macro local con valor `4096`; determina el tamaño del arreglo que conserva la
ruta generada para el archivo de llave.

### Funciones auxiliares

Todas estas funciones son `static` y solo se usan desde `main.c`.

#### `imprimir_uso`

```c
static void imprimir_uso(const char *programa);
```

Muestra en `stderr` las dos formas válidas de invocar el programa.

| Parámetro | Significado |
|---|---|
| `programa` | Nombre con que se ejecutó el binario, normalmente `argv[0]`. |

#### `leer_archivo`

```c
static int leer_archivo(const char *ruta, uint8_t **datos, size_t *longitud);
```

Abre un archivo en modo binario, reserva memoria y lee todo su contenido.
Devuelve el búfer y su longitud mediante parámetros de salida.

| Parámetro | Significado |
|---|---|
| `ruta` | Ruta del archivo que se leerá. |
| `datos` | Dirección de un puntero; al regresar con éxito apunta al búfer reservado. El llamador debe liberarlo con `free`. |
| `longitud` | Dirección de un `size_t` donde se guarda la cantidad de bytes leídos. |

Devuelve `0` si tuvo éxito y `-1` si falla al abrir, posicionar, reservar, leer
o cerrar el archivo. El archivo debe poder consultarse con `fseek` y `ftell`.

#### `misma_ruta_archivo`

```c
static int misma_ruta_archivo(const char *primera, const char *segunda);
```

Comprueba si dos rutas son iguales como texto o si apuntan al mismo archivo
existente, comparando dispositivo e inodo con `stat`.

| Parámetro | Significado |
|---|---|
| `primera` | Primera ruta. |
| `segunda` | Segunda ruta. |

Devuelve `1` si identifica el mismo archivo y `0` en caso contrario.

#### `escribir_hexadecimal`

```c
static int escribir_hexadecimal(FILE *archivo, const uint8_t *datos,
                                size_t longitud);
```

Escribe cada byte como dos dígitos hexadecimales en minúscula y termina la
línea con un salto de línea.

| Parámetro | Significado |
|---|---|
| `archivo` | Archivo abierto para escritura. |
| `datos` | Bytes que se van a codificar. |
| `longitud` | Cantidad de bytes que se codificará. |

Devuelve `0` si escribe todos los datos y `-1` si falla una escritura.

#### `cifrar_archivo`

```c
static int cifrar_archivo(const char *ruta_entrada, const char *ruta_salida);
```

Coordina el cifrado de un archivo. Rechaza que entrada y salida sean el mismo
archivo, lee la entrada, calcula el espacio para el relleno, genera la llave y
el IV, aplica CBC bloque por bloque y escribe cabecera, IV y cifrado
hexadecimal.

| Parámetro | Significado |
|---|---|
| `ruta_entrada` | Archivo original que se cifrará; su nombre base determina el nombre del archivo de llave. |
| `ruta_salida` | Archivo de texto hexadecimal que recibirá el resultado. |

Devuelve `0` si termina correctamente y `-1` si ocurre un error. La llave y el
texto se guardan en memoria mientras se procesa el archivo.

#### `descifrar_archivo`

```c
static int descifrar_archivo(const char *ruta_entrada,
                             const char *ruta_salida,
                             const char *ruta_llave);
```

Coordina el descifrado. Verifica el formato `DES-CBC-1`, convierte el IV y los
bloques hexadecimales a bytes, aplica DES en CBC, valida el relleno final y
escribe el contenido recuperado.

| Parámetro | Significado |
|---|---|
| `ruta_entrada` | Archivo cifrado en formato hexadecimal del programa. |
| `ruta_salida` | Archivo donde se escribirá el texto recuperado. |
| `ruta_llave` | Archivo `.env` de la misma llave que se usó al cifrar. |

Devuelve `0` si el descifrado y la escritura terminan correctamente; de lo
contrario devuelve `-1`. Un relleno inválido se informa como posible llave
incorrecta o archivo dañado; CBC por sí solo no permite autenticar cuál de esas
causas ocurrió.

#### `main`

```c
int main(int argc, char *argv[]);
```

Es el punto de entrada del programa. Revisa la cantidad y el valor de los
argumentos y llama a `cifrar_archivo` o `descifrar_archivo`.

| Parámetro | Significado |
|---|---|
| `argc` | Cantidad total de argumentos recibidos, incluido el nombre del programa. |
| `argv` | Arreglo de cadenas con los argumentos de consola. |

Devuelve `EXIT_SUCCESS` cuando la operación solicitada termina bien y
`EXIT_FAILURE` si la sintaxis es incorrecta o la operación falla.

## 8. Prueba: `tests/test_des.c`

#### `main`

La prueba usa la llave `133457799BBCDFF1` y el bloque de entrada
`0123456789ABCDEF`. Comprueba que cifrar produzca el resultado conocido
`85E813540F0AB405`, y después que descifrar el resultado recupere el bloque
original. `assert` termina la prueba si alguna comparación falla.

Para compilar y ejecutar la prueba desde la raíz del repositorio:

```bash
gcc -Wall -Wextra -Werror -std=c11 \
  proyecto-final/tests/test_des.c \
  proyecto-final/source/des.c \
  -o /tmp/test_des
/tmp/test_des
```

La salida esperada es:

```text
Prueba DES: correcta
```

## 9. Límites y errores esperables

- Se necesita acceso a `/dev/urandom`; el código usa llamadas POSIX como
  `open`, `read`, `write` y `close`, por lo que no es directamente portable a
  todos los sistemas operativos.
- La entrada completa se mantiene en memoria; archivos muy grandes pueden
  agotar la memoria disponible.
- El archivo de llave contiene el secreto en texto hexadecimal. Los permisos
  restringidos y `.gitignore` reducen exposiciones accidentales, pero no
  sustituyen una gestión segura de secretos.
- CBC no detecta de forma confiable modificaciones ni autentica el origen del
  archivo. No se debe interpretar el descifrado como prueba de autenticidad.
- Si la llave ya existe, se debe conservar y usar al descifrar. Para cifrar de
  nuevo un archivo con el mismo nombre, se necesita elegir una entrada con otro
  nombre o gestionar manualmente el archivo de llave anterior, asegurándose de
  que no se requiera para recuperar un cifrado previo.