#ifndef DES_H
#define DES_H

#include <stdint.h>

#define DES_TAM_BLOQUE 8

void des_cifrar_bloque(const uint8_t entrada[DES_TAM_BLOQUE],
                       uint8_t salida[DES_TAM_BLOQUE],
                       const uint8_t llave[DES_TAM_BLOQUE]);
void des_descifrar_bloque(const uint8_t entrada[DES_TAM_BLOQUE],
                          uint8_t salida[DES_TAM_BLOQUE],
                          const uint8_t llave[DES_TAM_BLOQUE]);

#endif
