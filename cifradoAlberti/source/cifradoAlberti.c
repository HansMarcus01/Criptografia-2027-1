// Programa que implementa el cifrado de Disco de Alberti leyendo desde un archivo .txt
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // 1. Verificar los argumentos de la terminal
    if (argc != 4) {
        printf("Uso: %s <archivo_entrada.txt> <posicion_inicial> <periodo>\n", argv[0]);
        return 1;
    }

    const char *inputfile = argv[1];
    int desplazamiento_inicial = atoi(argv[2]);
    int periodo = atoi(argv[3]);

    if (desplazamiento_inicial < 0 || desplazamiento_inicial > 25) {
        printf("Error: La posicion inicial debe estar entre 0 y 25.\n");
        return 1;
    }

    if (periodo < 1) {
        printf("Error: El periodo debe ser un numero mayor a 0.\n");
        return 1;
    }

    // 2. Abrir el archivo de texto
    FILE *file = fopen(inputfile, "r");
    if (file == NULL) {
        printf("No se pudo abrir el archivo %s\n", inputfile);
        return 1;
    }

    // 3. Obtener el tamaño del archivo para reservar memoria dinámica
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    if (fileSize == 0) {
        printf("El archivo esta vacio.\n");
        fclose(file);
        return 1;
    }

    // Reservar memoria para los arreglos de texto cifrado y descifrado
    char *texto = (char *)malloc((size_t)fileSize + 1);
    char *descifrado = (char *)malloc((size_t)fileSize + 1);
    
    if (texto == NULL || descifrado == NULL) {
        printf("No hay memoria suficiente para leer el archivo.\n");
        fclose(file);
        if (texto) free(texto);
        if (descifrado) free(descifrado);
        return 1;
    }

    // Leer el archivo completo y cerrar
    size_t leidos = fread(texto, 1, fileSize, file);
    texto[leidos] = '\0';
    fclose(file);

    int desplazamiento_actual = desplazamiento_inicial;
    int contador_letras = 0;

    // 4. Proceso de cifrado
    for (int i = 0; texto[i] != '\0'; i++) {
        if (isalpha(texto[i])) {
            if (isupper(texto[i])) {
                texto[i] = (char)('A' + (texto[i] - 'A' + desplazamiento_actual) % 26);
            } else if (islower(texto[i])) {
                texto[i] = (char)('a' + (texto[i] - 'a' + desplazamiento_actual) % 26);
            }

            contador_letras++;
            if (contador_letras % periodo == 0) {
                desplazamiento_actual = (desplazamiento_actual + 1) % 26;
            }
        }
    }

    // Copiar el texto cifrado para procesar el descifrado
    for (int i = 0; texto[i] != '\0'; i++) {
        descifrado[i] = texto[i];
        descifrado[i + 1] = '\0';
    }

    // 5. Proceso de descifrado
    desplazamiento_actual = desplazamiento_inicial;
    contador_letras = 0;

    for (int i = 0; descifrado[i] != '\0'; i++) {
        if (isalpha(descifrado[i])) {
            if (isupper(descifrado[i])) {
                descifrado[i] = (char)('A' + (descifrado[i] - 'A' - desplazamiento_actual + 26) % 26);
            } else if (islower(descifrado[i])) {
                descifrado[i] = (char)('a' + (descifrado[i] - 'a' - desplazamiento_actual + 26) % 26);
            }

            contador_letras++;
            if (contador_letras % periodo == 0) {
                desplazamiento_actual = (desplazamiento_actual + 1) % 26;
            }
        }
    }

    // Mostrar los resultados
    printf("--- MENSAJE CIFRADO ---\n%s\n", texto);
    printf("\n--- MENSAJE DESCIFRADO ---\n%s\n", descifrado);

    // Liberar la memoria reservada
    free(texto);
    free(descifrado);

    return 0;
}