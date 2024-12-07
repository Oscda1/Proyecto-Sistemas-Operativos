#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#define MAX_TAREA 256
#define ARCHIVO_TAREAS "tareas.txt"

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

void interfaz_usuario() {
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

int main() {
    interfaz_usuario();
    return 0;
}
