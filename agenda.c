#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TAREA 256
#define ARCHIVO_TAREAS "tareas.txt"

typedef struct {
  char fecha[11];
  char tarea[MAX_TAREA];
  int completada;
} Tarea;

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
    return;
  }

  int num_tarea = 1;
  char linea[MAX_TAREA + 20];
  while (fgets(linea, sizeof(linea), archivo)) {
    char *fecha = strtok(linea, "|");
    char *tarea = strtok(NULL, "|");
    char *completada = strtok(NULL, "|");

    printw("%d. Fecha: %s | Tarea: %s | Completada: %s\n", num_tarea, fecha,
           tarea, atoi(completada) ? "Si" : "No");
    num_tarea++;
  }

  fclose(archivo);
}

void actualizar_archivo(Tarea *tareas, int total_tareas) {
  FILE *archivo = fopen(ARCHIVO_TAREAS, "w");
  if (!archivo) {
    perror("Error al abrir el archivo de tareas");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < total_tareas; i++) {
    fprintf(archivo, "%s|%s|%d\n", tareas[i].fecha, tareas[i].tarea,
            tareas[i].completada);
  }

  fclose(archivo);
}

void cargar_tareas(Tarea *tareas, int *total_tareas) {
  FILE *archivo = fopen(ARCHIVO_TAREAS, "r");
  if (!archivo) {
    perror("Error al abrir el archivo de tareas");
    return;
  }

  char linea[MAX_TAREA + 20];
  *total_tareas = 0;
  while (fgets(linea, sizeof(linea), archivo) && *total_tareas < 100) {
    strcpy(tareas[*total_tareas].fecha, strtok(linea, "|"));
    strcpy(tareas[*total_tareas].tarea, strtok(NULL, "|"));
    tareas[*total_tareas].completada = atoi(strtok(NULL, "|"));
    (*total_tareas)++;
  }

  fclose(archivo);
}

void editar_tarea(Tarea *tareas, int total_tareas) {
  echo();
  int num_tarea;
  char nueva_fecha[11];
  char nueva_tarea[MAX_TAREA];
  int i, index = -1;

  printw("Ingrese el numero de la tarea a editar: ");
  refresh();
  scanw("%d", &num_tarea);

  if (num_tarea < 1 || num_tarea > total_tareas) {
    printw("Numero de tarea invalido.\n");
    refresh();
    return;
  }

  index = num_tarea - 1;

  printw("Tarea actual: %s\nFecha actual: %s\n", tareas[index].tarea,
         tareas[index].fecha);
  printw("Ingrese la nueva descripcion de la tarea: ");
  refresh();
  getstr(nueva_tarea);
  printw("Ingrese la nueva fecha (YYYY-MM-DD): ");
  refresh();
  getstr(nueva_fecha);

  strcpy(tareas[index].tarea, nueva_tarea);
  strcpy(tareas[index].fecha, nueva_fecha);

  actualizar_archivo(tareas, total_tareas);
  printw("Tarea editada exitosamente.\n");
  refresh();
  getch();
}

void eliminar_tarea(Tarea *tareas, int *total_tareas) {
  echo();
  int num_tarea;
  int i, j, index = -1;

  printw("Ingrese el numero de la tarea a eliminar: ");
  refresh();
  scanw("%d", &num_tarea);

  if (num_tarea < 1 || num_tarea > *total_tareas) {
    printw("Numero de tarea invalido.\n");
    refresh();
    return;
  }

  index = num_tarea - 1;

  for (j = index; j < *total_tareas - 1; j++) {
    tareas[j] = tareas[j + 1];
  }
  (*total_tareas)--;

  actualizar_archivo(tareas, *total_tareas);
  printw("Tarea eliminada exitosamente.\n");
  refresh();
  getch();
}

void completar_tarea(Tarea *tareas, int total_tareas) {
  echo();
  int num_tarea;
  int i, index = -1;

  printw("Ingrese el numero de la tarea a marcar como completada: ");
  refresh();
  scanw("%d", &num_tarea);

  if (num_tarea < 1 || num_tarea > total_tareas) {
    printw("Numero de tarea invalido.\n");
    refresh();
    return;
  }

  index = num_tarea - 1;
  tareas[index].completada = 1;

  actualizar_archivo(tareas, total_tareas);
  printw("Tarea marcada como completada.\n");
  refresh();
  getch();
}

void interfaz_usuario() {
  initscr();
  noecho();
  cbreak();

  Tarea tareas[100];
  int total_tareas;

  cargar_tareas(tareas, &total_tareas);

  while (1) {
    clear();
    printw("Agenda de Tareas\n");
    printw("1. Ver tareas\n");
    printw("2. Agregar tarea\n");
    printw("3. Editar tarea\n");
    printw("4. Eliminar tarea\n");
    printw("5. Marcar tarea como completada\n");
    printw("6. Salir\n");
    printw("Seleccione una opcion: ");
    refresh();

    int opcion = getch();
    switch (opcion) {
    case '1':
      clear();
      mostrar_tareas();
      printw("\nPresione cualquier tecla para continuar.");
      refresh();
      getch();
      break;
    case '2': {
      char fecha[11];
      char tarea[MAX_TAREA];

      echo();
      clear();
      printw("Ingrese la fecha limite (DD/MM/YYYY): ");
      refresh();
      getstr(fecha);
      printw("Ingrese la descripcion de la tarea: ");
      refresh();
      getstr(tarea);
      noecho();

      agregar_tarea(fecha, tarea);
      printw("Tarea agregada exitosamente.\n");
      printw("Presione cualquier tecla para continuar.");
      refresh();
      getch();
      cargar_tareas(tareas, &total_tareas);
      break;
    }
    case '3':
      clear();
      editar_tarea(tareas, total_tareas);
      break;
    case '4':
      clear();
      eliminar_tarea(tareas, &total_tareas);
      break;
    case '5':
      clear();
      completar_tarea(tareas, total_tareas);
      break;
    case '6':
      endwin();
      return;
    default:
      printw("Opcion invalida. Intentelo de nuevo.\n");
      refresh();
      getch();
      break;
    }
  }

  endwin();
}

int main() {
  interfaz_usuario();
  return 0;
}
