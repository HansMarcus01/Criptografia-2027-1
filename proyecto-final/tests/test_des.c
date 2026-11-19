#include "../source/des.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    const uint8_t llave[DES_TAM_BLOQUE] = {
        0x13, 0x34, 0x57, 0x79, 0x9b, 0xbc, 0xdf, 0xf1};
    const uint8_t texto_claro[DES_TAM_BLOQUE] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
    const uint8_t esperado[DES_TAM_BLOQUE] = {
        0x85, 0xe8, 0x13, 0x54, 0x0f, 0x0a, 0xb4, 0x05};
    uint8_t cifrado[DES_TAM_BLOQUE];
    uint8_t descifrado[DES_TAM_BLOQUE];

    des_cifrar_bloque(texto_claro, cifrado, llave);
    assert(cifrado[0] == esperado[0] && cifrado[1] == esperado[1] &&
           cifrado[2] == esperado[2] && cifrado[3] == esperado[3] &&
           cifrado[4] == esperado[4] && cifrado[5] == esperado[5] &&
           cifrado[6] == esperado[6] && cifrado[7] == esperado[7]);
    des_descifrar_bloque(cifrado, descifrado, llave);
    for (size_t i = 0; i < DES_TAM_BLOQUE; i++) {
        assert(descifrado[i] == texto_claro[i]);
    }
    puts("Prueba DES: correcta");
    return 0;
}
