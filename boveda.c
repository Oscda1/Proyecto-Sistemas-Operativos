#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_LLAVE 64
#define BUFFER_SIZE 1024

void cifrar_archivo(const char *ruta, const char *llave) {
    FILE *archivo = fopen(ruta, "rb");
    if (!archivo) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    FILE *archivo_cifrado = fopen("archivo_cifrado", "wb");
    if (!archivo_cifrado) {
        perror("Error al crear el archivo cifrado");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    uint8_t buffer[BUFFER_SIZE];
    size_t bytes_leidos;
    size_t longitud_llave = strlen(llave);

    while ((bytes_leidos = fread(buffer, 1, BUFFER_SIZE, archivo)) > 0) {
        for (size_t i = 0; i < bytes_leidos; i++) {
            buffer[i] ^= llave[i % longitud_llave]; // Enmascaramiento XOR
            buffer[i] = (buffer[i] << 1) | (buffer[i] >> 7); // Desplazamiento bit a bit
            buffer[i] += 1; // Suma
        }
        fwrite(buffer, 1, bytes_leidos, archivo_cifrado);
    }

    fclose(archivo);
    fclose(archivo_cifrado);
}

void descifrar_archivo(const char *ruta, const char *llave) {
    FILE *archivo_cifrado = fopen(ruta, "rb");
    if (!archivo_cifrado) {
        perror("Error al abrir el archivo cifrado");
        exit(EXIT_FAILURE);
    }

    FILE *archivo_descifrado = fopen("archivo_descifrado", "wb");
    if (!archivo_descifrado) {
        perror("Error al crear el archivo descifrado");
        fclose(archivo_cifrado);
        exit(EXIT_FAILURE);
    }

    uint8_t buffer[BUFFER_SIZE];
    size_t bytes_leidos;
    size_t longitud_llave = strlen(llave);

    while ((bytes_leidos = fread(buffer, 1, BUFFER_SIZE, archivo_cifrado)) > 0) {
        for (size_t i = 0; i < bytes_leidos; i++) {
            buffer[i] -= 1; // Resta
            buffer[i] = (buffer[i] >> 1) | (buffer[i] << 7); // Desplazamiento bit a bit inverso
            buffer[i] ^= llave[i % longitud_llave]; // Enmascaramiento XOR inverso
        }
        fwrite(buffer, 1, bytes_leidos, archivo_descifrado);
    }

    fclose(archivo_cifrado);
    fclose(archivo_descifrado);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s /boveda [ruta] [llave]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *comando = argv[1];
    const char *ruta = argv[2];
    const char *llave = argv[3];

    if (strlen(llave) > MAX_LLAVE) {
        fprintf(stderr, "La llave privada no debe exceder los %d bytes\n", MAX_LLAVE);
        exit(EXIT_FAILURE);
    }

    if (strcmp(comando, "/boveda") == 0) {
        cifrar_archivo(ruta, llave);
        printf("Archivo cifrado exitosamente.\n");
    } else {
        fprintf(stderr, "Comando no reconocido.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
