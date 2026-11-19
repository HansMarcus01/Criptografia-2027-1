#ifndef GENERADOR_LLAVES_H
#define GENERADOR_LLAVES_H

#include <stddef.h>
#include <stdint.h>

#define LLAVE_DES_TAM 8

int obtener_bytes_aleatorios(uint8_t *destino, size_t cantidad);
int generar_archivo_llave(const char *archivo_entrada,
                          uint8_t llave[LLAVE_DES_TAM],
                          char *ruta_llave, size_t capacidad_ruta);
int leer_archivo_llave(const char *ruta_llave,
                       uint8_t llave[LLAVE_DES_TAM]);

#endif
