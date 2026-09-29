#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

static void imprimir_uso(const char *programa) {
    fprintf(stderr,
            "Uso: %s -m <c|d> -i <entrada> -o <salida> -k <clave> [-v]\n"
            "Opciones requeridas:\n"
            "  -m <c|d> : Modo (c: cifrar, d: descifrar)\n"
            "  -i <archivo> : Archivo de entrada\n"
            "  -o <archivo> : Archivo de salida\n"
            "  -k <clave> : Clave de 4 letras (matriz 2x2) o 9 letras (matriz 3x3)\n"
            "Opciones opcionales:\n"
            "  -v : Imprime el proceso matricial en la consola\n",
            programa);
}

/* Operacion modulo real que maneja correctamente numeros negativos */
static int mod(int a, int b) {
    return (a % b + b) % b;
}

/* Maximo Comun Divisor */
static int gcd(int a, int b) {
    while (b) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

/* Inverso modular extendido */
static int modInverse(int a, int m) {
    a = mod(a, m);
    for (int x = 1; x < m; x++) {
        if (mod(a * x, m) == 1) {
            return x;
        }
    }
    return -1;
}

/* Cofactor de una matriz 3x3 para calcular la matriz adjunta */
static int cofactor(int K[3][3], int f, int c) {
    int sub[2][2];
    int r2 = 0;
    for (int i = 0; i < 3; i++) {
        if (i == f) continue;
        int c2 = 0;
        for (int j = 0; j < 3; j++) {
            if (j == c) continue;
            sub[r2][c2] = K[i][j];
            c2++;
        }
        r2++;
    }
    int det = sub[0][0] * sub[1][1] - sub[0][1] * sub[1][0];
    return ((f + c) % 2 == 0) ? det : -det;
}

int main(int argc, char *argv[]) {
    int opt;
    char modo = 0;
    char *archivo_entrada = NULL;
    char *archivo_salida = NULL;
    char *clave_str = NULL;
    int verbose = 0;

    while ((opt = getopt(argc, argv, "m:i:o:k:v")) != -1) {
        switch (opt) {
            case 'm': modo = optarg[0]; break;
            case 'i': archivo_entrada = optarg; break;
            case 'o': archivo_salida = optarg; break;
            case 'k': clave_str = optarg; break;
            case 'v': verbose = 1; break;
            default: imprimir_uso(argv[0]); return EXIT_FAILURE;
        }
    }

    if (modo == 0 || archivo_entrada == NULL || archivo_salida == NULL || clave_str == NULL) {
        imprimir_uso(argv[0]);
        return EXIT_FAILURE;
    }

    if (modo != 'c' && modo != 'd') {
        fprintf(stderr, "Error: el modo debe ser 'c' o 'd'.\n");
        return EXIT_FAILURE;
    }

    size_t len_clave = strlen(clave_str);
    int N = 0;
    if (len_clave == 4) N = 2;
    else if (len_clave == 9) N = 3;
    else {
        fprintf(stderr, "Error: La clave de Hill debe ser de 4 letras (2x2) o 9 letras (3x3).\n");
        return EXIT_FAILURE;
    }

    /* Construir Matriz K */
    int K[3][3] = {0};
    for (size_t i = 0; i < len_clave; i++) {
        if (!isalpha(clave_str[i])) {
            fprintf(stderr, "Error: La clave solo debe contener letras.\n");
            return EXIT_FAILURE;
        }
        K[i / N][i % N] = toupper(clave_str[i]) - 'A';
    }

    /* Invertir matriz si estamos en modo descifrado */
    if (modo == 'd') {
        int det = 0;
        int Adj[3][3] = {0};

        if (N == 2) {
            det = K[0][0] * K[1][1] - K[0][1] * K[1][0];
            Adj[0][0] = K[1][1];
            Adj[0][1] = -K[0][1];
            Adj[1][0] = -K[1][0];
            Adj[1][1] = K[0][0];
        } else {
            for (int i = 0; i < 3; i++) {
                det += K[0][i] * cofactor(K, 0, i);
            }
            /* La adjunta es la transpuesta de los cofactores */
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    Adj[j][i] = cofactor(K, i, j);
                }
            }
        }

        det = mod(det, 26);
        if (gcd(det, 26) != 1) {
            fprintf(stderr, "Error: La clave es invalida. Su determinante (%d) no es coprimo con 26. No se puede invertir.\n", det);
            return EXIT_FAILURE;
        }

        int invDet = modInverse(det, 26);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                K[i][j] = mod(Adj[i][j] * invDet, 26);
            }
        }
    }

    /* Leer archivo a memoria e indexar letras */
    FILE *entrada = fopen(archivo_entrada, "rb");
    if (!entrada) {
        fprintf(stderr, "Error abriendo '%s'.\n", archivo_entrada);
        return EXIT_FAILURE;
    }

    size_t cap_texto = 1024, cap_idx = 1024;
    char *texto = malloc(cap_texto);
    size_t *indices = malloc(cap_idx * sizeof(size_t));
    size_t len_texto = 0, num_letras = 0;

    int c_char;
    while ((c_char = fgetc(entrada)) != EOF) {
        if (len_texto >= cap_texto) { cap_texto *= 2; texto = realloc(texto, cap_texto); }
        texto[len_texto] = c_char;

        if (isalpha(c_char)) {
            if (num_letras >= cap_idx) { cap_idx *= 2; indices = realloc(indices, cap_idx * sizeof(size_t)); }
            indices[num_letras++] = len_texto;
        }
        len_texto++;
    }
    fclose(entrada);

    /* Padding de Hill: Si el total de letras no cuadra con el bloque N, rellenar con 'X' */
    while (num_letras % N != 0) {
        if (len_texto >= cap_texto) { cap_texto *= 2; texto = realloc(texto, cap_texto); }
        texto[len_texto] = 'X';
        if (num_letras >= cap_idx) { cap_idx *= 2; indices = realloc(indices, cap_idx * sizeof(size_t)); }
        indices[num_letras++] = len_texto++;
    }

    if (verbose) printf("--- PROCESANDO POR BLOQUES DE %dx%d ---\n", N, N);

    /* Aplicar multiplicacion matricial a bloques de texto */
    for (size_t b = 0; b < num_letras; b += N) {
        int vector[3];
        int res[3] = {0};
        char original_case[3];

        for (int i = 0; i < N; i++) {
            char t = texto[indices[b + i]];
            original_case[i] = isupper(t) ? 'A' : 'a';
            vector[i] = toupper(t) - 'A';
        }

        /* Multiplicacion: res = K * vector */
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                res[i] = mod(res[i] + K[i][j] * vector[j], 26);
            }
        }

        /* Escritura del resultado en memoria, respetando la capitalizacion original */
        if (verbose) printf("Bloque: [");
        for (int i = 0; i < N; i++) {
            if (verbose) printf("%c(%2d)%s", toupper(texto[indices[b+i]]), vector[i], (i == N-1) ? "] -> [" : ", ");
            texto[indices[b + i]] = res[i] + original_case[i];
        }
        if (verbose) {
            for (int i = 0; i < N; i++) {
                printf("%c(%2d)%s", res[i] + 'A', res[i], (i == N-1) ? "]\n" : ", ");
            }
        }
    }

    FILE *salida = fopen(archivo_salida, "wb");
    if (!salida) {
        fprintf(stderr, "Error creando '%s'.\n", archivo_salida);
        free(texto); free(indices);
        return EXIT_FAILURE;
    }

    fwrite(texto, 1, len_texto, salida);
    fclose(salida);
    free(texto);
    free(indices);

    if (verbose) printf("--- COMPLETADO ---\n");
    return EXIT_SUCCESS;
}