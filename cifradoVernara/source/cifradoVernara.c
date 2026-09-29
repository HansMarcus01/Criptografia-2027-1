#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* Requerido para getopt */

static void imprimir_uso(const char *programa) {
	fprintf(stderr,
			"Uso: %s -m <c|d> -i <entrada> -o <salida> -k <clave> [-v]\n"
			"Opciones requeridas:\n"
			"  -m <c|d> : Modo a ejecutar (c: cifrar, d: descifrar)\n"
			"  -i <archivo> : Archivo de entrada\n"
			"  -o <archivo> : Archivo de salida\n"
			"  -k <clave> : Cadena de letras (A-Z) para la operacion XOR de 5 bits\n"
			"Opciones opcionales:\n"
			"  -v : Imprime el proceso paso a paso en la consola\n",
			programa);
}

static int validar_clave(const char *clave) {
	if (strlen(clave) == 0) {
		return 0;
	}
	/* La clave debe contener unicamente letras del alfabeto */
	for (size_t i = 0; clave[i] != '\0'; i++) {
		char c = clave[i];
		if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) {
			return 0;
		}
	}
	return 1;
}

/* Funcion auxiliar para convertir un numero (0-31) a binario de 5 bits */
static void obtener_binario(int valor, char *buffer) {
	for (int i = 4; i >= 0; i--) {
		buffer[4 - i] = (valor & (1 << i)) ? '1' : '0';
	}
	buffer[5] = '\0';
}

int main(int argc, char *argv[]) {
	int caracter;
	FILE *entrada;
	FILE *salida;
	size_t len_clave;
	size_t idx_clave = 0;

	/* Variables para el manejo de argumentos CLI */
	int opt;
	char modo = 0;
	char *archivo_entrada = NULL;
	char *archivo_salida = NULL;
	char *clave = NULL;
	int verbose = 0;

	/* Parseo de argumentos estandar con getopt */
	while ((opt = getopt(argc, argv, "m:i:o:k:v")) != -1) {
		switch (opt) {
			case 'm':
				if (strcmp(optarg, "c") == 0 || strcmp(optarg, "d") == 0) {
					modo = optarg[0];
				} else {
					fprintf(stderr, "Error: el modo (-m) debe ser 'c' o 'd'.\n");
					return EXIT_FAILURE;
				}
				break;
			case 'i':
				archivo_entrada = optarg;
				break;
			case 'o':
				archivo_salida = optarg;
				break;
			case 'k':
				clave = optarg;
				break;
			case 'v':
				verbose = 1;
				break;
			default:
				imprimir_uso(argv[0]);
				return EXIT_FAILURE;
		}
	}

	/* Validar que todos los parametros obligatorios se hayan proporcionado */
	if (modo == 0 || archivo_entrada == NULL || archivo_salida == NULL || clave == NULL) {
		imprimir_uso(argv[0]);
		return EXIT_FAILURE;
	}

	if (!validar_clave(clave)) {
		fprintf(stderr, "Error: la clave debe contener solo letras (A-Z).\n");
		return EXIT_FAILURE;
	}
	if (strcmp(archivo_entrada, archivo_salida) == 0) {
		fprintf(stderr, "Error: entrada y salida deben ser archivos distintos.\n");
		return EXIT_FAILURE;
	}

	len_clave = strlen(clave);

	entrada = fopen(archivo_entrada, "rb");
	if (entrada == NULL) {
		fprintf(stderr, "Error: no se pudo abrir '%s'.\n", archivo_entrada);
		return EXIT_FAILURE;
	}
	salida = fopen(archivo_salida, "wb");
	if (salida == NULL) {
		fprintf(stderr, "Error: no se pudo crear '%s'.\n", archivo_salida);
		fclose(entrada);
		return EXIT_FAILURE;
	}

	if (verbose) {
		printf("--- INICIANDO PROCESAMIENTO (Modo: %c) ---\n", modo);
	}

	while ((caracter = fgetc(entrada)) != EOF) {
		char char_upper = caracter;
		/* Normalizar a mayuscula si es una letra */
		if (caracter >= 'a' && caracter <= 'z') {
			char_upper = caracter - 'a' + 'A';
		}

		/* Procesar si esta dentro del rango expandido de 5 bits (0-31 valores). */
		if (char_upper >= 'A' && char_upper <= 'A' + 31) {
			int p = char_upper - 'A';

			char char_clave = clave[idx_clave];
			if (char_clave >= 'a' && char_clave <= 'z') {
				char_clave = char_clave - 'a' + 'A';
			}
			int k = char_clave - 'A';

			int c = p ^ k;

			if (verbose) {
				char bin_p[6], bin_k[6], bin_c[6];
				obtener_binario(p, bin_p);
				obtener_binario(k, bin_k);
				obtener_binario(c, bin_c);

				printf("Texto: '%c' (%2d) [%s] XOR Clave: '%c' (%2d) [%s] -> Res: '%c' (%2d) [%s]\n",
						char_upper, p, bin_p,
						char_clave, k, bin_k,
						c + 'A', c, bin_c);
			}

			if (fputc(c + 'A', salida) == EOF) {
				fprintf(stderr, "Error al escribir '%s'.\n", archivo_salida);
				fclose(entrada);
				fclose(salida);
				return EXIT_FAILURE;
			}

			idx_clave = (idx_clave + 1) % len_clave;
		} else {
			if (verbose && caracter != '\n' && caracter != '\r') {
				printf("Ignorando caracter especial: '%c'\n", caracter);
			}
			if (fputc(caracter, salida) == EOF) {
				fprintf(stderr, "Error al escribir '%s'.\n", archivo_salida);
				fclose(entrada);
				fclose(salida);
				return EXIT_FAILURE;
			}
		}
	}

	if (ferror(entrada) || fclose(entrada) != 0 || fclose(salida) != 0) {
		fprintf(stderr, "Error al cerrar los archivos.\n");
		return EXIT_FAILURE;
	}

	if (verbose) {
		printf("--- PROCESAMIENTO FINALIZADO ---\n");
	}

	return EXIT_SUCCESS;
}