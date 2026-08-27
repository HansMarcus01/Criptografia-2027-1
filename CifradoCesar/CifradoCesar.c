// Programa que implementa el cifrado César en C sin librerías de cifrado.
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    char texto[100];
    char descifrado[100];
    int desplazamiento, valido = 1;
    int manual = 0;

    printf("Ingrese el texto a cifrar: ");
    if (scanf(" %99[^\n]", texto) != 1) {
        printf("No se pudo leer el mensaje.\n");
        return 1;
    }
    printf("Desea ingresar manualmente la llave de cifrado? (1 para sí, 0 para no): ");
    if (scanf("%d", &manual) != 1) {
        printf("No se pudo leer la opción.\n");
        return 1;
    }

    if (manual) {
        printf("Ingrese la llave de cifrado (0-25): ");
        if (scanf("%d", &desplazamiento) != 1) {
            printf("No se pudo leer la llave.\n");
            return 1;
        }
        if (desplazamiento < 0 || desplazamiento > 25) {
            printf("La llave debe estar entre 0 y 25.\n");
            return 1;
        }
    } else {
        srand((unsigned int)time(NULL));
        desplazamiento = rand() % 26;
    }

    for (int i = 0; texto[i] != '\0'; i++) {
        if (!((texto[i] >= 'A' && texto[i] <= 'Z') ||
              (texto[i] >= 'a' && texto[i] <= 'z') || texto[i] == ' ')) {
            valido = 0;
            break;
        }
    }

    if (!valido) {
        printf("Error: use solamente letras del alfabeto ingles (A-Z) y espacios.\n");
        return 1;
    }

    for (int i = 0; texto[i] != '\0'; i++) {
        if (texto[i] >= 'A' && texto[i] <= 'Z') {
            texto[i] = (char)('A' + (texto[i] - 'A' + desplazamiento) % 26);
        } else if (texto[i] >= 'a' && texto[i] <= 'z') {
            texto[i] = (char)('a' + (texto[i] - 'a' + desplazamiento) % 26);
        }
    }

    for (int i = 0; texto[i] != '\0'; i++) {
        descifrado[i] = texto[i];
        descifrado[i + 1] = '\0';
    }

    for (int i = 0; descifrado[i] != '\0'; i++) {
        if (descifrado[i] >= 'A' && descifrado[i] <= 'Z') {
            descifrado[i] = (char)('A' + (descifrado[i] - 'A' - desplazamiento + 26) % 26);
        } else if (descifrado[i] >= 'a' && descifrado[i] <= 'z') {
            descifrado[i] = (char)('a' + (descifrado[i] - 'a' - desplazamiento + 26) % 26);
        }
    }

    printf("Llave generada: %d\n", desplazamiento);
    printf("Mensaje cifrado: %s\n", texto);
    printf("Mensaje descifrado: %s\n", descifrado);

    return 0;
}