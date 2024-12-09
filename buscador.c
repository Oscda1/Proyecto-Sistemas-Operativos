#include <ctype.h> // Para isprint()
#include <curses.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_COMMANDS 1000
#define MAX_COMMAND_LENGTH 100
#define MAX_LINES 50 // Número máximo de líneas que se pueden mostrar en la pantalla
#define BOTTOM_TEXT " [^] DESPLAZARTE ARRIBA [v] DESPLAZARTE ABAJO [Enter] SELECCIONAR [Texto] BUSCAR [Esc] Salir"

// Función para cargar los comandos desde un archivo
void load_commands_from_file(const char *filename, char *commands[], int *num_commands) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_COMMAND_LENGTH];
    ssize_t bytes_read;
    *num_commands = 0;

    while ((bytes_read = read(fd, buffer, MAX_COMMAND_LENGTH - 1)) > 0) {
        buffer[bytes_read] = '\0'; // Asegurar terminación
        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            commands[*num_commands] = strdup(line);
            (*num_commands)++;
            line = strtok(NULL, "\n");
        }
    }
    close(fd);
}

// Función para filtrar comandos según la búsqueda
void filter_commands(char *commands[], int num_commands, char *search_term,
                     char *filtered_commands[], int *num_filtered) {
    *num_filtered = 0;
    for (int i = 0; i < num_commands; i++) {
        if (search_term[0] == '\0' || strstr(commands[i], search_term) != NULL) {
            filtered_commands[*num_filtered] = commands[i];
            (*num_filtered)++;
        }
    }
}

// Función para mostrar los comandos en pantalla
void display_commands(WINDOW *win, char *commands[], int num_commands,
                      int selected, int start_line, char *search_term) {
    int i, y;
    int lines_to_show =
        MAX_LINES - 1; // Dejamos la última línea para el texto de instrucciones

    clear(); // Limpiar la pantalla

    // Mostrar la línea de búsqueda en la parte superior
    mvprintw(0, 0, "Buscar: %s", search_term);

    // Mostrar la lista de comandos, con un límite de líneas en la pantalla
    for (i = start_line, y = 1; i < num_commands && y < lines_to_show; i++, y++) {
        if (i == selected) {
            attron(A_REVERSE); // Resaltar el comando seleccionado
        }
        mvprintw(y, 0, "%s", commands[i]);
        if (i == selected) {
            attroff(A_REVERSE); // Apagar el resaltado
        }
    }

    // Mostrar el texto de instrucciones en la parte inferior
    mvprintw(MAX_LINES - 1, 0, BOTTOM_TEXT); // Imprimir texto en la última línea
    refresh();
}

// Función para mostrar el manual de un comando
void show_man_page(char *command) {
    if (fork() == 0) {
        char *args[] = {"man", command, NULL};
        execvp("man", args);
        perror("execvp failed");
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
        exit(EXIT_SUCCESS); // Termina el proceso después de mostrar el manual
    }
}

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    char *commands[MAX_COMMANDS];
    int num_commands = 0;

    // Cargar los comandos desde el archivo "comandos"
    load_commands_from_file("comandos", commands, &num_commands);

    char *filtered_commands[MAX_COMMANDS];
    int num_filtered = 0;
    char search_term[MAX_COMMAND_LENGTH] = ""; // Cadena de búsqueda inicial vacía

    int selected = 0;
    int start_line = 0; // Línea inicial de la lista a mostrar
    int max_lines =
        MAX_LINES - 1; // Reservamos una línea para el texto de instrucciones

    // Filtrar los comandos inicialmente para mostrar todos
    filter_commands(commands, num_commands, search_term, filtered_commands,
                    &num_filtered);
    display_commands(stdscr, filtered_commands, num_filtered, selected,
                     start_line, search_term);

    int running = 1; // Variable para controlar el bucle principal
    while (running) {
        int ch = getch(); // Capturar la tecla presionada
        switch (ch) {
        case KEY_UP:
            if (selected > 0) {
                selected--;
                if (selected < start_line) {
                    start_line--; // Mover la pantalla hacia arriba
                }
            }
            break;
        case KEY_DOWN:
            if (selected < num_filtered - 1) {
                selected++;
                if (selected >= start_line + max_lines) {
                    start_line++; // Mover la pantalla hacia abajo
                }
            }
            break;
        case 10: // Enter key
            show_man_page(filtered_commands[selected]);
            break;
        case 27:       // Escape key to quit
            printf("\033[H\033[J");
            running = 0; // Termina el bucle principal
            break;
        case KEY_BACKSPACE: // Backspace key, borrar el último carácter de la búsqueda
            if (strlen(search_term) > 0) {
                search_term[strlen(search_term) - 1] = '\0';
            }
            // Reiniciar selección y línea inicial
            selected = 0;
            start_line = 0;
            break;
        default:
            // Añadir el carácter al término de búsqueda (si es alfanumérico)
            if (isprint(ch)) {
                int len = strlen(search_term);
                if (len < MAX_COMMAND_LENGTH - 1) {
                    search_term[len] = (char)ch;
                    search_term[len + 1] = '\0';
                }
                // Reiniciar selección y línea inicial
                selected = 0;
                start_line = 0;
            }
            break;
        }

        // Filtrar comandos según la búsqueda
        filter_commands(commands, num_commands, search_term, filtered_commands,
                        &num_filtered);

        display_commands(stdscr, filtered_commands, num_filtered, selected,
                         start_line, search_term);
    }
    endwin(); // Finalizar ncurses
    return 0;
}
