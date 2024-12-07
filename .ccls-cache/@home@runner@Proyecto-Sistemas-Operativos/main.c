#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ncurses.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINEA 1024
#define MAX_LLAVE 64
#define BUFFER_SIZE 1024
#define MAX_TAREA 256
#define ARCHIVO_TAREAS "tareas.txt"
#define MAX_COMANDO 256
#define ARCHIVO_COMANDOS "comandos.txt"

// Funciones del shell
void ejecutar_comando(char *comando) {
    char *argumentos[MAX_LINEA / 2 + 1];
    int i = 0;

    argumentos[i] = strtok(comando, " \n");
    while (argumentos[i] != NULL) {
        i++;
        argumentos[i] = strtok(NULL, " \n");
    }

    if (fork() == 0) {
        execvp(argumentos[0], argumentos);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
    }
}

// Funciones de cifrado
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

// Funciones de la agenda
void agregar_tarea(const char *fecha, const char *tarea) {
    FILE *archivo = fopen(ARCHIVO_TAREAS, "a");
    if (!archivo) {
        perror("Error al abrir el archivo de tareas");
        exit(EXIT_FAILURE);
    }
    fprintf(archivo, "%s|%s|0\n", fecha, tarea);
    fclose(archivo);
}

void mostrar_tareas() {
    FILE *archivo = fopen(ARCHIVO_TAREAS, "r");
    if (!archivo) {
        perror("Error al abrir el archivo de tareas");
        exit(EXIT_FAILURE);
    }

    char linea[MAX_TAREA + 20];
    while (fgets(linea, sizeof(linea), archivo)) {
        char *fecha = strtok(linea, "|");
        char *tarea = strtok(NULL, "|");
        char *completada = strtok(NULL, "|");

        printw("Fecha: %s | Tarea: %s | Completada: %s\n", fecha, tarea, atoi(completada) ? "Sí" : "No");
    }

    fclose(archivo);
}

void interfaz_agenda() {
    initscr();
    noecho();
    cbreak();

    while (1) {
        clear();
        printw("Agenda de Tareas\n");
        printw("1. Ver tareas\n");
        printw("2. Agregar tarea\n");
        printw("3. Salir\n");
        printw("Seleccione una opción: ");
        refresh();

        int opcion = getch();
        if (opcion == '1') {
            clear();
            mostrar_tareas();
            printw("\nPresione cualquier tecla para continuar...");
            refresh();
            getch();
        } else if (opcion == '2') {
            char fecha[11];
            char tarea[MAX_TAREA];

            echo();
            clear();
            printw("Ingrese la fecha (YYYY-MM-DD): ");
            refresh();
            getstr(fecha);
            printw("Ingrese la tarea: ");
            refresh();
            getstr(tarea);
            noecho();

            agregar_tarea(fecha, tarea);
            printw("Tarea agregada exitosamente.\n");
            printw("Presione cualquier tecla para continuar...");
            refresh();
            getch();
        } else if (opcion == '3') {
            break;
        }
    }

    endwin();
}

// Funciones de búsqueda de comandos
void indexar_comandos() {
    FILE *archivo = fopen(ARCHIVO_COMANDOS, "w");
    if (!archivo) {
        perror("Error al abrir el archivo de comandos");
        exit(EXIT_FAILURE);
    }

    char *path = getenv("PATH");
    char *directorio = strtok(path, ":");
    struct dirent *entrada;

    while (directorio != NULL) {
        DIR *dir = opendir(directorio);
        if (dir) {
            while ((entrada = readdir(dir)) != NULL) {
                if (entrada->d_type == DT_REG) {
                    fprintf(archivo, "%s\n", entrada->d_name);
                }
            }
            closedir(dir);
        }
        directorio = strtok(NULL, ":");
    }

    fclose(archivo);
}

void mostrar_manual(const char *comando) {
    char comando_man[MAX_COMANDO];
    snprintf(comando_man, sizeof(comando_man), "man %s", comando);
    endwin();
    system(comando_man);
    initscr();
    noecho();
    cbreak();
}

void interfaz_comandos() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    FILE *archivo = fopen(ARCHIVO_COMANDOS, "r");
    if (!archivo) {
        perror("Error al abrir el archivo de comandos");
        exit(EXIT_FAILURE);
    }

    char comandos[1000][MAX_COMANDO];
    int num_comandos = 0;
    while (fgets(comandos[num_comandos], MAX_COMANDO, archivo)) {
        comandos[num_comandos][strcspn(comandos[num_comandos], "\n")] = '\0';
        num_comandos++;
    }
    fclose(archivo);

    int seleccion = 0;
    while (1) {
        clear();
        for (int i = 0; i < num_comandos; i++) {
            if (i == seleccion) {
                attron(A_REVERSE);
            }
            mvprintw(i, 0, "%s", comandos[i]);
            if (i == seleccion) {
                attroff(A_REVERSE);
            }
        }
        refresh();

        int ch = getch();
        if (ch == KEY_UP) {
            seleccion = (seleccion - 1 + num_comandos) % num_comandos;
        } else if (ch == KEY_DOWN) {
            seleccion = (seleccion + 1) % num_comandos;
        } else if (ch == '\n') {
            mostrar_manual(comandos[seleccion]);
        } else if (ch == 'q') {
            break;
        }
    }

    endwin();
}

// Función principal
int main() {
    if (access(ARCHIVO_COMANDOS, F_OK) == -1) {
        indexar_comandos();
    }

    char comando[MAX_LINEA];

    while (1) {
        printf("shell> ");
        if (fgets(comando, MAX_LINEA, stdin) == NULL) {
            break;
        }

        if (strcmp(comando, "salir\n") == 0) {
            break;
        } else if (strncmp(comando, "/boveda", 7) == 0) {
            char *ruta = strtok(comando + 8, " ");
            char *llave = strtok(NULL, " \n");
            if (ruta && llave) {
                cifrar_archivo(ruta, llave);
                printf("Archivo cifrado exitosamente.\n");
            } else {
                fprintf(stderr, "Uso: /boveda [ruta] [llave]\n");
            }
         } else if (strncmp(comando, "/agenda", 7) == 0) {
            interfaz_agenda();
         } else if (strncmp(comando, "/?", 2) == 0) {
            interfaz_comandos();
         } else {
            ejecutar_comando(comando);
         }
     }
    
 return 0;
}