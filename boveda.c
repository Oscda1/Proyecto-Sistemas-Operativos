#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Función para encriptar/desencriptar con control de rango
void encryptDecrypt(char *input, const char *key, size_t lenInput) {
    size_t lenKey = strlen(key);
    for (size_t i = 0; i < lenInput; i++) {
        input[i] = (unsigned char)((input[i] ^ key[i % lenKey]) % 256);
    }
}

// Función para cargar el contenido del archivo en memoria
char *loadFile(const char *filename, size_t *fileSize) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error al abrir el archivo");
        return NULL;
    }

    // Obtener el tamaño del archivo
    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    rewind(file);

    // Reservar memoria para el contenido
    char *content = (char *)malloc(*fileSize + 1);
    if (!content) {
        perror("Error al asignar memoria");
        fclose(file);
        return NULL;
    }

    // Leer el contenido del archivo
    fread(content, 1, *fileSize, file);
    fclose(file);
    content[*fileSize] = '\0'; // Asegurar terminación de cadena

    return content;
}

// Función para guardar contenido en un archivo
int saveFile(const char *filename, const char *content, size_t fileSize) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error al escribir en el archivo");
        return 0;
    }
    fwrite(content, 1, fileSize, file);
    fclose(file);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s -e|-d archivo clave\n", argv[0]);
        return 1;
    }

    const char *option = argv[1];
    const char *filename = argv[2];
    const char *key = argv[3];

    // Validar que la opción es correcta
    if (strcmp(option, "-e") != 0 && strcmp(option, "-d") != 0) {
        fprintf(stderr, "Opción no válida. Use -e para encriptar o -d para desencriptar.\n");
        return 1;
    }

    // Cargar el contenido del archivo
    size_t fileSize;
    char *content = loadFile(filename, &fileSize);
    if (!content) {
        return 1;
    }

    if (strcmp(option, "-e") == 0) {
        // Encriptar el archivo y sobrescribir
        encryptDecrypt(content, key, fileSize);
        if (saveFile(filename, content, fileSize)) {
            printf("Archivo encriptado y sobrescrito con éxito.\n");
        }
    } else if (strcmp(option, "-d") == 0) {
        // Desencriptar el archivo y mostrar en pantalla
        char *decryptedContent = (char *)malloc(fileSize + 1);
        if (!decryptedContent) {
            perror("Error al asignar memoria");
            free(content);
            return 1;
        }
        memcpy(decryptedContent, content, fileSize);
        encryptDecrypt(decryptedContent, key, fileSize);
        decryptedContent[fileSize] = '\0'; // Asegurar terminación
        printf("%s\n", decryptedContent);
        free(decryptedContent);
    }

    // Liberar memoria
    free(content);
    return 0;
}

