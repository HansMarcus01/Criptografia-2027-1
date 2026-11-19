#include "des.h"
#include "generador_llaves.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define TAM_RUTA 4096

static void imprimir_uso(const char *programa) {
    fprintf(stderr,
            "Uso:\n"
            "  %s c <entrada> <salida>\n"
            "  %s d <entrada_cifrada> <salida> <archivo_llave>\n",
            programa, programa);
}

static int leer_archivo(const char *ruta, uint8_t **datos, size_t *longitud) {
    FILE *archivo = fopen(ruta, "rb");
    long tamano;
    uint8_t *buffer;

    if (archivo == NULL) {
        return -1;
    }
    if (fseek(archivo, 0, SEEK_END) != 0 ||
        (tamano = ftell(archivo)) < 0 || fseek(archivo, 0, SEEK_SET) != 0) {
        int error = errno == 0 ? EIO : errno;
        fclose(archivo);
        errno = error;
        return -1;
    }
    buffer = malloc(tamano == 0 ? 1 : (size_t)tamano);
    if (buffer == NULL) {
        fclose(archivo);
        return -1;
    }
    if ((size_t)tamano > 0 &&
        fread(buffer, 1, (size_t)tamano, archivo) != (size_t)tamano) {
        int error = ferror(archivo) ? errno : EIO;
        free(buffer);
        fclose(archivo);
        errno = error == 0 ? EIO : error;
        return -1;
    }
    if (fclose(archivo) != 0) {
        int error = errno;
        free(buffer);
        errno = error;
        return -1;
    }
    *datos = buffer;
    *longitud = (size_t)tamano;
    return 0;
}

static int misma_ruta_archivo(const char *primera, const char *segunda) {
    struct stat estado_primero;
    struct stat estado_segundo;

    if (strcmp(primera, segunda) == 0) {
        return 1;
    }
    return stat(primera, &estado_primero) == 0 &&
           stat(segunda, &estado_segundo) == 0 &&
           estado_primero.st_dev == estado_segundo.st_dev &&
           estado_primero.st_ino == estado_segundo.st_ino;
}

static int escribir_hexadecimal(FILE *archivo, const uint8_t *datos,
                                size_t longitud) {
    for (size_t i = 0; i < longitud; i++) {
        if (fprintf(archivo, "%02x", datos[i]) < 0) {
            return -1;
        }
    }
    return fputc('\n', archivo) == EOF ? -1 : 0;
}

static int cifrar_archivo(const char *ruta_entrada, const char *ruta_salida) {
    uint8_t llave[LLAVE_DES_TAM];
    uint8_t vector[DES_TAM_BLOQUE];
    uint8_t *texto = NULL;
    uint8_t *cifrado = NULL;
    size_t longitud_texto;
    size_t longitud_cifrado;
    char ruta_llave[TAM_RUTA];
    FILE *salida = NULL;
    int estado = -1;

    if (misma_ruta_archivo(ruta_entrada, ruta_salida)) {
        fprintf(stderr, "Error: entrada y salida deben ser archivos distintos.\n");
        return -1;
    }
    if (leer_archivo(ruta_entrada, &texto, &longitud_texto) != 0) {
        perror("Error al leer la entrada");
        return -1;
    }
    if (longitud_texto > SIZE_MAX - DES_TAM_BLOQUE) {
        fprintf(stderr, "Error: el archivo de entrada es demasiado grande.\n");
        goto fin;
    }
    longitud_cifrado =
        (longitud_texto / DES_TAM_BLOQUE + 1) * DES_TAM_BLOQUE;
    cifrado = malloc(longitud_cifrado);
    if (cifrado == NULL) {
        perror("Error al reservar memoria");
        goto fin;
    }
    if (generar_archivo_llave(ruta_entrada, llave, ruta_llave,
                              sizeof(ruta_llave)) != 0) {
        perror("Error al generar el archivo de llave");
        goto fin;
    }
    if (obtener_bytes_aleatorios(vector, sizeof(vector)) != 0) {
        perror("Error al generar el vector de inicializacion");
        goto fin;
    }

    uint8_t anterior[DES_TAM_BLOQUE];
    memcpy(anterior, vector, sizeof(anterior));
    for (size_t desplazamiento = 0; desplazamiento < longitud_cifrado;
         desplazamiento += DES_TAM_BLOQUE) {
        uint8_t bloque[DES_TAM_BLOQUE];
        size_t restantes = longitud_texto > desplazamiento
                               ? longitud_texto - desplazamiento
                               : 0;
        size_t cantidad = restantes < DES_TAM_BLOQUE
                              ? restantes
                              : DES_TAM_BLOQUE;
        uint8_t relleno = (uint8_t)(longitud_cifrado - longitud_texto);

        if (cantidad > 0) {
            memcpy(bloque, texto + desplazamiento, cantidad);
        }
        memset(bloque + cantidad, relleno, DES_TAM_BLOQUE - cantidad);
        for (size_t i = 0; i < DES_TAM_BLOQUE; i++) {
            bloque[i] ^= anterior[i];
        }
        des_cifrar_bloque(bloque, cifrado + desplazamiento, llave);
        memcpy(anterior, cifrado + desplazamiento, sizeof(anterior));
    }

    salida = fopen(ruta_salida, "wb");
    if (salida == NULL) {
        perror("Error al crear el archivo de salida");
        goto fin;
    }
    int error_escritura = fprintf(salida, "DES-CBC-1\nIV=") < 0;
    if (!error_escritura &&
        (escribir_hexadecimal(salida, vector, sizeof(vector)) != 0 ||
         escribir_hexadecimal(salida, cifrado, longitud_cifrado) != 0)) {
        error_escritura = 1;
    }
    if (fclose(salida) != 0) {
        error_escritura = 1;
    }
    salida = NULL;
    if (error_escritura) {
        perror("Error al escribir el archivo cifrado");
        goto fin;
    }
    printf("Archivo cifrado: %s\nLlave guardada en: %s\n", ruta_salida,
           ruta_llave);
    estado = 0;

fin:
    if (salida != NULL) {
        fclose(salida);
    }
    free(cifrado);
    free(texto);
    return estado;
}

static int descifrar_archivo(const char *ruta_entrada, const char *ruta_salida,
                             const char *ruta_llave) {
    static const char cabecera[] = "DES-CBC-1\nIV=";
    uint8_t llave[LLAVE_DES_TAM];
    uint8_t vector[DES_TAM_BLOQUE];
    uint8_t *archivo = NULL;
    uint8_t *cifrado = NULL;
    uint8_t *texto = NULL;
    size_t longitud_archivo;
    size_t inicio_hex;
    size_t longitud_hex;
    size_t longitud_cifrado;
    FILE *salida = NULL;
    int estado = -1;

    if (misma_ruta_archivo(ruta_entrada, ruta_salida) ||
        misma_ruta_archivo(ruta_llave, ruta_salida)) {
        fprintf(stderr, "Error: la salida debe ser distinta de la entrada y la llave.\n");
        return -1;
    }
    if (leer_archivo_llave(ruta_llave, llave) != 0) {
        perror("Error al leer el archivo de llave");
        return -1;
    }
    if (leer_archivo(ruta_entrada, &archivo, &longitud_archivo) != 0) {
        perror("Error al leer la entrada cifrada");
        return -1;
    }
    if (longitud_archivo < sizeof(cabecera) - 1 + 16 + 1 ||
        memcmp(archivo, cabecera, sizeof(cabecera) - 1) != 0) {
        fprintf(stderr, "Error: el archivo no tiene el formato DES-CBC esperado.\n");
        goto fin;
    }
    inicio_hex = sizeof(cabecera) - 1;
    for (size_t i = 0; i < sizeof(vector); i++) {
        int alto;
        int bajo;
        char a = (char)archivo[inicio_hex + i * 2];
        char b = (char)archivo[inicio_hex + i * 2 + 1];

        alto = a >= '0' && a <= '9' ? a - '0'
               : a >= 'a' && a <= 'f' ? a - 'a' + 10
               : a >= 'A' && a <= 'F' ? a - 'A' + 10
                                       : -1;
        bajo = b >= '0' && b <= '9' ? b - '0'
               : b >= 'a' && b <= 'f' ? b - 'a' + 10
               : b >= 'A' && b <= 'F' ? b - 'A' + 10
                                       : -1;
        if (alto < 0 || bajo < 0) {
            fprintf(stderr, "Error: vector de inicializacion invalido.\n");
            goto fin;
        }
        vector[i] = (uint8_t)((alto << 4) | bajo);
    }
    inicio_hex += sizeof(vector) * 2;
    if (archivo[inicio_hex++] != '\n') {
        fprintf(stderr, "Error: formato invalido despues del vector de inicializacion.\n");
        goto fin;
    }
    longitud_hex = longitud_archivo - inicio_hex;
    if (longitud_hex > 0 && archivo[longitud_archivo - 1] == '\n') {
        longitud_hex--;
    }
    if (longitud_hex == 0 || longitud_hex % 2 != 0) {
        fprintf(stderr, "Error: texto cifrado hexadecimal invalido.\n");
        goto fin;
    }
    longitud_cifrado = longitud_hex / 2;
    if (longitud_cifrado % DES_TAM_BLOQUE != 0) {
        fprintf(stderr, "Error: longitud del texto cifrado invalida.\n");
        goto fin;
    }
    cifrado = malloc(longitud_cifrado);
    texto = malloc(longitud_cifrado);
    if (cifrado == NULL || texto == NULL) {
        perror("Error al reservar memoria");
        goto fin;
    }
    for (size_t i = 0; i < longitud_cifrado; i++) {
        char a = (char)archivo[inicio_hex + i * 2];
        char b = (char)archivo[inicio_hex + i * 2 + 1];
        int alto = a >= '0' && a <= '9' ? a - '0'
                   : a >= 'a' && a <= 'f' ? a - 'a' + 10
                   : a >= 'A' && a <= 'F' ? a - 'A' + 10
                                           : -1;
        int bajo = b >= '0' && b <= '9' ? b - '0'
                   : b >= 'a' && b <= 'f' ? b - 'a' + 10
                   : b >= 'A' && b <= 'F' ? b - 'A' + 10
                                           : -1;

        if (alto < 0 || bajo < 0) {
            fprintf(stderr, "Error: texto cifrado hexadecimal invalido.\n");
            goto fin;
        }
        cifrado[i] = (uint8_t)((alto << 4) | bajo);
    }
    for (size_t desplazamiento = 0; desplazamiento < longitud_cifrado;
         desplazamiento += DES_TAM_BLOQUE) {
        uint8_t bloque[DES_TAM_BLOQUE];
        const uint8_t *anterior =
            desplazamiento == 0 ? vector : cifrado + desplazamiento - DES_TAM_BLOQUE;

        des_descifrar_bloque(cifrado + desplazamiento, bloque, llave);
        for (size_t i = 0; i < DES_TAM_BLOQUE; i++) {
            texto[desplazamiento + i] = bloque[i] ^ anterior[i];
        }
    }

    uint8_t relleno = texto[longitud_cifrado - 1];
    if (relleno == 0 || relleno > DES_TAM_BLOQUE) {
        fprintf(stderr, "Error: llave incorrecta o relleno invalido.\n");
        goto fin;
    }
    for (size_t i = 0; i < relleno; i++) {
        if (texto[longitud_cifrado - 1 - i] != relleno) {
            fprintf(stderr, "Error: llave incorrecta o relleno invalido.\n");
            goto fin;
        }
    }

    salida = fopen(ruta_salida, "wb");
    if (salida == NULL) {
        perror("Error al crear el archivo de salida");
        goto fin;
    }
    int error_escritura =
        fwrite(texto, 1, longitud_cifrado - relleno, salida) !=
        longitud_cifrado - relleno;
    if (fclose(salida) != 0) {
        error_escritura = 1;
    }
    salida = NULL;
    if (error_escritura) {
        perror("Error al escribir el archivo descifrado");
        goto fin;
    }
    printf("Archivo descifrado: %s\n", ruta_salida);
    estado = 0;

fin:
    if (salida != NULL) {
        fclose(salida);
    }
    free(texto);
    free(cifrado);
    free(archivo);
    return estado;
}

int main(int argc, char *argv[]) {
    if (argc == 4 && strcmp(argv[1], "c") == 0) {
        return cifrar_archivo(argv[2], argv[3]) == 0 ? EXIT_SUCCESS
                                                      : EXIT_FAILURE;
    }
    if (argc == 5 && strcmp(argv[1], "d") == 0) {
        return descifrar_archivo(argv[2], argv[3], argv[4]) == 0
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }
    imprimir_uso(argv[0]);
    return EXIT_FAILURE;
}
