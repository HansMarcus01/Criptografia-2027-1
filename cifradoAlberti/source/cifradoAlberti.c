// Cifrado de rueda de Alberti con una rueda externa fija y una interna claveada.
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALFABETO_TAM 26

static void imprimir_uso(const char *programa) {
    fprintf(stderr,
            "Uso: %s <c|d> <entrada> <salida> <posicion> <periodo> <+|-> <clave>\n"
            "  c: cifrar, d: descifrar\n"
            "  posicion: giro inicial de 0 a 25\n"
            "  periodo: letras entre giros, mayor que 0\n"
            "  clave: permutacion de 26 letras ASCII sin repetir\n",
            programa);
}

static int parsear_entero(const char *texto, long minimo, long maximo,
                          long *resultado) {
    char *fin;

    errno = 0;
    *resultado = strtol(texto, &fin, 10);
    return errno == 0 && *texto != '\0' && *fin == '\0' &&
           *resultado >= minimo && *resultado <= maximo;
}

static int validar_clave(const char *clave) {
    int vistos[ALFABETO_TAM] = {0};

    if (strlen(clave) != ALFABETO_TAM) {
        return 0;
    }
    for (size_t i = 0; i < ALFABETO_TAM; i++) {
        if (clave[i] < 'A' || clave[i] > 'Z' || vistos[clave[i] - 'A']) {
            return 0;
        }
        vistos[clave[i] - 'A'] = 1;
    }
    return 1;
}

static char transformar(char caracter, int cifrar, const char *rueda_interna,
                        int posicion) {
    int es_mayuscula = caracter >= 'A' && caracter <= 'Z';
    int es_minuscula = caracter >= 'a' && caracter <= 'z';
    int indice;
    char resultado;

    if (!es_mayuscula && !es_minuscula) {
        return caracter;
    }

    indice = es_mayuscula ? caracter - 'A' : caracter - 'a';
    if (cifrar) {
        resultado = rueda_interna[(indice + posicion) % ALFABETO_TAM];
    } else {
        int indice_interno = 0;
        char mayuscula = es_mayuscula ? caracter : (char)(caracter - 'a' + 'A');

        while (rueda_interna[indice_interno] != mayuscula) {
            indice_interno++;
        }
        resultado = (char)('A' + (indice_interno - posicion + ALFABETO_TAM) %
                           ALFABETO_TAM);
    }
    return es_mayuscula ? resultado : (char)(resultado - 'A' + 'a');
}

int main(int argc, char *argv[]) {
    const char *archivo_entrada;
    const char *archivo_salida;
    const char *clave;
    long posicion_inicial;
    long periodo;
    int contador_letras = 0;
    int posicion_actual;
    FILE *entrada;
    FILE *salida;
    int caracter;

    if (argc != 8 || (strcmp(argv[1], "c") != 0 && strcmp(argv[1], "d") != 0) ||
        (strcmp(argv[6], "+") != 0 && strcmp(argv[6], "-") != 0)) {
        imprimir_uso(argv[0]);
        return EXIT_FAILURE;
    }
    if (!parsear_entero(argv[4], 0, 25, &posicion_inicial)) {
        fprintf(stderr, "Error: la posicion debe estar entre 0 y 25.\n");
        return EXIT_FAILURE;
    }
    if (!parsear_entero(argv[5], 1, 2147483647, &periodo)) {
        fprintf(stderr, "Error: el periodo debe ser un entero mayor que 0.\n");
        return EXIT_FAILURE;
    }
    if (!validar_clave(argv[7])) {
        fprintf(stderr, "Error: la clave debe ser una permutacion de A-Z sin repetir.\n");
        return EXIT_FAILURE;
    }

    archivo_entrada = argv[2];
    archivo_salida = argv[3];
    clave = argv[7];
    if (strcmp(archivo_entrada, archivo_salida) == 0) {
        fprintf(stderr, "Error: entrada y salida deben ser archivos distintos.\n");
        return EXIT_FAILURE;
    }

    entrada = fopen(archivo_entrada, "rb");
    if (entrada == NULL) {
        fprintf(stderr, "Error: no se pudo abrir '%s'.\n", archivo_entrada);
        return EXIT_FAILURE;
    }
    salida = fopen(archivo_salida, "wb");
    if (salida == NULL) {
        fprintf(stderr, "Error: no se pudo crear '%s'.\n", archivo_salida);
        fclose(entrada);
        return EXIT_FAILURE;
    }

    posicion_actual = (int)posicion_inicial;
    while ((caracter = fgetc(entrada)) != EOF) {
        int es_letra = (caracter >= 'A' && caracter <= 'Z') ||
                       (caracter >= 'a' && caracter <= 'z');
        char transformado = transformar((char)caracter, argv[1][0] == 'c',
                                        clave, posicion_actual);

        if (fputc((unsigned char)transformado, salida) == EOF) {
            fprintf(stderr, "Error al escribir '%s'.\n", archivo_salida);
            fclose(entrada);
            fclose(salida);
            return EXIT_FAILURE;
        }
        if (es_letra) {
            contador_letras++;
            if (contador_letras % periodo == 0) {
                int giro = argv[6][0] == '+' ? 1 : -1;
                posicion_actual = (posicion_actual + giro + ALFABETO_TAM) %
                                  ALFABETO_TAM;
            }
        }
    }

    int error_lectura = ferror(entrada);
    int error_entrada = fclose(entrada) != 0;
    int error_salida = fclose(salida) != 0;
    if (error_lectura || error_entrada || error_salida) {
        fprintf(stderr, "Error al cerrar los archivos.\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
