#include "generador_llaves.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int escribir_todo(int descriptor, const char *datos, size_t cantidad) {
    size_t escritos = 0;

    while (escritos < cantidad) {
        ssize_t resultado = write(descriptor, datos + escritos, cantidad - escritos);

        if (resultado < 0 && errno == EINTR) {
            continue;
        }
        if (resultado <= 0) {
            return -1;
        }
        escritos += (size_t)resultado;
    }
    return 0;
}

int obtener_bytes_aleatorios(uint8_t *destino, size_t cantidad) {
    int descriptor = open("/dev/urandom", O_RDONLY);
    size_t recibidos = 0;

    if (descriptor < 0) {
        return -1;
    }
    while (recibidos < cantidad) {
        ssize_t resultado = read(descriptor, destino + recibidos,
                                 cantidad - recibidos);

        if (resultado < 0 && errno == EINTR) {
            continue;
        }
        if (resultado <= 0) {
            int error = resultado == 0 ? EIO : errno;
            close(descriptor);
            errno = error;
            return -1;
        }
        recibidos += (size_t)resultado;
    }
    if (close(descriptor) != 0) {
        return -1;
    }
    return 0;
}

static void ajustar_paridad_impar(uint8_t llave[LLAVE_DES_TAM]) {
    for (size_t i = 0; i < LLAVE_DES_TAM; i++) {
        unsigned int unos = 0;

        llave[i] &= 0xfe;
        for (unsigned int bit = 1; bit < 8; bit++) {
            unos += (llave[i] >> bit) & 1U;
        }
        if (unos % 2 == 0) {
            llave[i] |= 1;
        }
    }
}

static int construir_ruta_llave(const char *archivo_entrada,
                                char *ruta_llave, size_t capacidad_ruta) {
    const char *separador = strrchr(archivo_entrada, '/');
    const char *nombre = separador == NULL ? archivo_entrada : separador + 1;
    size_t longitud_prefijo = separador == NULL
                                  ? 0
                                  : (size_t)(separador - archivo_entrada) + 1;
    int resultado;

    if (*nombre == '\0') {
        errno = EINVAL;
        return -1;
    }
    resultado = snprintf(ruta_llave, capacidad_ruta, "%.*sllaves%s.env",
                         (int)longitud_prefijo, archivo_entrada, nombre);
    if (resultado < 0 || (size_t)resultado >= capacidad_ruta) {
        errno = ENAMETOOLONG;
        return -1;
    }
    return 0;
}

int generar_archivo_llave(const char *archivo_entrada,
                          uint8_t llave[LLAVE_DES_TAM],
                          char *ruta_llave, size_t capacidad_ruta) {
    static const char prefijo[] = "DES_KEY=";
    char contenido[sizeof(prefijo) - 1 + LLAVE_DES_TAM * 2 + 2];
    size_t longitud = sizeof(prefijo) - 1;
    int descriptor;

    if (construir_ruta_llave(archivo_entrada, ruta_llave, capacidad_ruta) != 0 ||
        obtener_bytes_aleatorios(llave, LLAVE_DES_TAM) != 0) {
        return -1;
    }
    ajustar_paridad_impar(llave);
    memcpy(contenido, prefijo, sizeof(prefijo) - 1);
    for (size_t i = 0; i < LLAVE_DES_TAM; i++) {
        int resultado = snprintf(contenido + longitud,
                                 sizeof(contenido) - longitud, "%02x", llave[i]);

        if (resultado != 2) {
            errno = EIO;
            return -1;
        }
        longitud += 2;
    }
    contenido[longitud++] = '\n';

    descriptor = open(ruta_llave, O_WRONLY | O_CREAT | O_EXCL,
                      S_IRUSR | S_IWUSR);
    if (descriptor < 0) {
        return -1;
    }
    if (escribir_todo(descriptor, contenido, longitud) != 0) {
        int error = errno;
        close(descriptor);
        unlink(ruta_llave);
        errno = error;
        return -1;
    }
    if (close(descriptor) != 0) {
        int error = errno;
        unlink(ruta_llave);
        errno = error;
        return -1;
    }
    return 0;
}

static int valor_hexadecimal(char caracter) {
    if (caracter >= '0' && caracter <= '9') {
        return caracter - '0';
    }
    if (caracter >= 'a' && caracter <= 'f') {
        return caracter - 'a' + 10;
    }
    if (caracter >= 'A' && caracter <= 'F') {
        return caracter - 'A' + 10;
    }
    return -1;
}

int leer_archivo_llave(const char *ruta_llave,
                       uint8_t llave[LLAVE_DES_TAM]) {
    static const char prefijo[] = "DES_KEY=";
    char linea[64];
    FILE *archivo = fopen(ruta_llave, "r");
    size_t longitud;
    int siguiente;

    if (archivo == NULL) {
        return -1;
    }
    if (fgets(linea, sizeof(linea), archivo) == NULL) {
        fclose(archivo);
        errno = EINVAL;
        return -1;
    }
    longitud = strlen(linea);
    if (longitud > 0 && linea[longitud - 1] == '\n') {
        linea[--longitud] = '\0';
    }
    if (longitud != sizeof(prefijo) - 1 + LLAVE_DES_TAM * 2 ||
        memcmp(linea, prefijo, sizeof(prefijo) - 1) != 0) {
        fclose(archivo);
        errno = EINVAL;
        return -1;
    }
    for (size_t i = 0; i < LLAVE_DES_TAM; i++) {
        int alto = valor_hexadecimal(linea[sizeof(prefijo) - 1 + i * 2]);
        int bajo = valor_hexadecimal(linea[sizeof(prefijo) + i * 2]);

        if (alto < 0 || bajo < 0) {
            fclose(archivo);
            errno = EINVAL;
            return -1;
        }
        llave[i] = (uint8_t)((alto << 4) | bajo);
    }
    siguiente = fgetc(archivo);
    if (ferror(archivo) || siguiente != EOF) {
        fclose(archivo);
        errno = EINVAL;
        return -1;
    }
    if (fclose(archivo) != 0) {
        return -1;
    }
    return 0;
}
