/* Cifrado digrafico de Wheatstone con una cuadracula de 5 x 5. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CUADRADO_TAM 25
#define ALFABETO "ABCDEFGHIKLMNOPQRSTUVWXYZ"

static void imprimir_uso(const char *programa) {
	fprintf(stderr,
			"Uso: %s <c|d> <entrada> <salida> <clave>\n"
			"  c: cifrar, d: descifrar\n"
			"  clave: permutacion de 25 letras de A-Z, sin J ni repetir\n",
			programa);
}

static int validar_clave(const char *clave) {
	int vistos[26] = {0};

	if (strlen(clave) != CUADRADO_TAM) {
		return 0;
	}
	for (size_t indice = 0; indice < CUADRADO_TAM; indice++) {
		if (clave[indice] < 'A' || clave[indice] > 'Z' || clave[indice] == 'J' || vistos[clave[indice] - 'A']) {
			return 0;
		}
		vistos[clave[indice] - 'A'] = 1;
	}
	return 1;
}

static void construir_cuadrado(const char *clave, char *cuadrado) {
	int usados[26] = {0};
	size_t posicion = 0;

	for (size_t indice = 0; indice < CUADRADO_TAM; indice++) {
		cuadrado[posicion++] = clave[indice];
		usados[clave[indice] - 'A'] = 1;
	}
	for (size_t indice = 0; ALFABETO[indice] != '\0'; indice++) {
		int letra = ALFABETO[indice] - 'A';
		if (!usados[letra]) {
			cuadrado[posicion++] = ALFABETO[indice];
		}
	}
}

static void imprimir_cuadrado(const char *cuadrado) {
	printf("Matriz de letras:\n");
	for (int fila = 0; fila < 5; fila++) {
		for (int columna = 0; columna < 5; columna++) {
			printf("%c ", cuadrado[fila * 5 + columna]);
		}
		putchar('\n');
	}
}

static int indice_de(char caracter, const char *cuadrado) {
	char normalizado = caracter >= 'a' && caracter <= 'z' ? (char)(caracter - 'a' + 'A') : caracter;
	normalizado = normalizado == 'J' ? 'I' : normalizado;

	for (int indice = 0; indice < CUADRADO_TAM; indice++) {
		if (cuadrado[indice] == normalizado) {
			return indice;
		}
	}
	return -1;
}

static char letra_de(int indice, const char *cuadrado, int mayuscula) {
	char resultado = cuadrado[indice];

	return mayuscula ? resultado : (char)(resultado - 'A' + 'a');
}

static void transformar_par(char primero, char segundo, int cifrar, const char *cuadrado, char *resultado_primero, char *resultado_segundo) {
	int indice_primero = indice_de(primero, cuadrado);
	int indice_segundo = indice_de(segundo, cuadrado);
	int fila_primero = indice_primero / 5;
	int columna_primero = indice_primero % 5;
	int fila_segundo = indice_segundo / 5;
	int columna_segundo = indice_segundo % 5;
	int resultado_indice_primero;
	int resultado_indice_segundo;

	if (fila_primero == fila_segundo) {
		int giro = cifrar ? 1 : -1;
		resultado_indice_primero = fila_primero * 5 +
								   (columna_primero + giro + 5) % 5;
		resultado_indice_segundo = fila_segundo * 5 +
								   (columna_segundo + giro + 5) % 5;
	} else if (columna_primero == columna_segundo) {
		int giro = cifrar ? 1 : -1;
		resultado_indice_primero =
			((fila_primero + giro + 5) % 5) * 5 + columna_primero;
		resultado_indice_segundo =
			((fila_segundo + giro + 5) % 5) * 5 + columna_segundo;
	} else {
		resultado_indice_primero = fila_primero * 5 + columna_segundo;
		resultado_indice_segundo = fila_segundo * 5 + columna_primero;
	}

	*resultado_primero = letra_de(resultado_indice_primero, cuadrado, primero >= 'A' && primero <= 'Z');
	*resultado_segundo = letra_de(resultado_indice_segundo, cuadrado, segundo >= 'A' && segundo <= 'Z');
}

int main(int argc, char *argv[]) {
	int pendiente = 0;
	int caracter;
	FILE *entrada;
	FILE *salida;
	char cuadrado[CUADRADO_TAM];

	if (argc != 5 || (strcmp(argv[1], "c") != 0 && strcmp(argv[1], "d") != 0)) {
		imprimir_uso(argv[0]);
		return EXIT_FAILURE;
	}
	if (!validar_clave(argv[4])) {
		fprintf(stderr, "Error: la clave debe tener 25 letras mayusculas, sin J ni repetir.\n");
		return EXIT_FAILURE;
	}
	if (strcmp(argv[2], argv[3]) == 0) {
		fprintf(stderr, "Error: entrada y salida deben ser archivos distintos.\n");
		return EXIT_FAILURE;
	}

	construir_cuadrado(argv[4], cuadrado);
	imprimir_cuadrado(cuadrado);
	entrada = fopen(argv[2], "rb");
	if (entrada == NULL) {
		fprintf(stderr, "Error: no se pudo abrir '%s'.\n", argv[2]);
		return EXIT_FAILURE;
	}
	salida = fopen(argv[3], "wb");
	if (salida == NULL) {
		fprintf(stderr, "Error: no se pudo crear '%s'.\n", argv[3]);
		fclose(entrada);
		return EXIT_FAILURE;
	}

	while ((caracter = fgetc(entrada)) != EOF) {
		int es_letra = (caracter >= 'A' && caracter <= 'Z') || (caracter >= 'a' && caracter <= 'z');

		if (!es_letra) {
			if (pendiente != 0) {
				if (fputc((unsigned char)pendiente, salida) == EOF) {
                    fprintf(stderr, "Error al escribir '%s'.\n", argv[3]);
					fclose(entrada);
					fclose(salida);
					return EXIT_FAILURE;
				}
				pendiente = 0;
			}
			if (fputc((unsigned char)caracter, salida) == EOF) {
				fprintf(stderr, "Error al escribir '%s'.\n", argv[3]);
				fclose(entrada);
				fclose(salida);
				return EXIT_FAILURE;
			}
			continue;
		}

		if (caracter == 'J' || caracter == 'j') {
			caracter = caracter == 'J' ? 'I' : 'i';
		}
		if (pendiente == 0) {
			pendiente = caracter;
		} else {
			char primero;
			char segundo;
			transformar_par((char)pendiente, (char)caracter, argv[1][0] == 'c', cuadrado, &primero, &segundo);
			if (fputc((unsigned char)primero, salida) == EOF || fputc((unsigned char)segundo, salida) == EOF) {
				fprintf(stderr, "Error al escribir '%s'.\n", argv[3]);
				fclose(entrada);
				fclose(salida);
				return EXIT_FAILURE;
			}
			pendiente = 0;
		}
	}

	if (pendiente != 0) {
		if (fputc((unsigned char)pendiente, salida) == EOF) {
			fprintf(stderr, "Error al escribir '%s'.\n", argv[3]);
			fclose(entrada);
			fclose(salida);
			return EXIT_FAILURE;
		}
	}

	if (ferror(entrada) || fclose(entrada) != 0 || fclose(salida) != 0) {
		fprintf(stderr, "Error al cerrar los archivos.\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}