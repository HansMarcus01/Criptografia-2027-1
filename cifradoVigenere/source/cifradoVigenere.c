#include<stdio.h>
#include<string.h>
#include <ctype.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Uso: %s <key> <inputfile.txt>\n", argv[0]);
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

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *encryptedText = (char *)malloc((size_t)fileSize + 1);
    if (encryptedText == NULL) {
        printf("No hay memoria suficiente para leer el archivo.\n");
        fclose(file);
        return 1;
    }

    int encryptIndex = 0;
    int decryptIndex = 0;
    int position = 0;
    int c;

    while ((c = fgetc(file)) != EOF && position < (int)fileSize) {
        if (isalpha(c)) {
            int shift = toupper(key[encryptIndex % keyLength]) - 'A';

            if (isupper(c)) {
                c = ((c - 'A' + shift) % 26) + 'A';
            }
            else if (islower(c)) {
                c = ((c - 'a' + shift) % 26) + 'a';
            }

            encryptIndex++;
        }

        encryptedText[position] = (char)c;
        position++;
    }
    encryptedText[position] = '\0';
    fclose(file);

    printf("Texto cifrado: ");
    int i = 0;
    while (encryptedText[i] != '\0') {
        putchar(encryptedText[i]);
        i++;
    }
    printf("\n");

    // 3) Descifrar la cadena almacenada, carácter por carácter
    printf("Texto descifrado: ");
    i = 0;
    while (encryptedText[i] != '\0') {
        c = encryptedText[i];

        if (isalpha(c)) {
            int shift = toupper(key[decryptIndex % keyLength]) - 'A';

            if (isupper(c)) {
                c = ((c - 'A' - shift + 26) % 26) + 'A';
            }
            else if (islower(c)) {
                c = ((c - 'a' - shift + 26) % 26) + 'a';
            }

            decryptIndex++;
        }

        putchar(c);
        i++;
    }

    printf("\n");
    free(encryptedText);
    return 0;
}
