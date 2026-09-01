#include<stdio.h>
#include<string.h>
#include <ctype.h>
int main(int argc, char *argv[]){
    if(argc != 3){
        printf("""Uso: %s <key> <inputfile.txt>\n", argv[0]);
        return 1;
    }
    const char *key = argv[1];
    const char *inputfile = argv[2];

    int keyLength = 0;
    while(key[keyLength] != '\0'){
        keyLength++;
    }

    if (keyLength == 0) {
        printf("La llave no puede estar vacía.\n");
        return 1;
    }

    FILE *file = fopen(inputfile, "r");
    if (file == NULL) {
        printf("No se pudo abrir el archivo %s\n", inputfile);
        return 1;
    }

    int j = 0;
    int c;

    while ((c = fgetc(file)) != EOF) {

        // Use ctype.h to check if the character is a letter
        if (isalpha(c)) {
            // toupper() ensures the key works regardless of its case
            int shift = toupper(key[j % keyLength]) - 'A';

            // Encrypt while preserving the original case
            if (isupper(c)) {
                c = ((c - 'A' + shift) % 26) + 'A';
            }
            else if (islower(c)) {
                c = ((c - 'a' + shift) % 26) + 'a';
            }
            // Advance the key index only when a letter is encrypted
            j++;
        }
        // Print the character (encrypted if it was a letter, unchanged otherwise)
        putchar(c);
    }

    // 5. Clean up
    fclose(file);
    printf("\n");

    return 0;
}

