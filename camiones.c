#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define CAMION_UABC "UABC"
#define CAMION_NATURA "NATURA"
#define META 50 // Distancia corta para la carrera

// Gráfico ASCII de los camiones
const char *uabc_bus = "_______________  \n"
                       "|__|__|__|__|__|___ \n"
                       "|    UABC          |\n"
                       "|~~~@~~~~~~~~~@~~~|)\n";

const char *natura_bus = "_______________  \n"
                         "|__|__|__|__|__|___ \n"
                         "|    NATURA        |\n"
                         "|~~~@~~~~~~~~~@~~~|)\n";

// Funcion para mover un camion aleatoriamente
int mover_camion() {
  return rand() % 3; // El camion puede avanzar 0, 1 o 2 espacios aleatoriamente
}

void dibujar_carrera(int pos_uabc, int pos_natura) {
  // Dibujar el camión de UABC
  for (int i = 0; i < pos_uabc; i++) {
    printw(" "); // Espacios para mover el camión a la derecha
  }
  printw("_______________  \n");
  for (int i = 0; i < pos_uabc; i++) {
    printw(" ");
  }
  printw("|__|__|__|__|__|___ \n");
  for (int i = 0; i < pos_uabc; i++) {
    printw(" ");
  }
  printw("|    UABC           |\n");
  for (int i = 0; i < pos_uabc; i++) {
    printw(" ");
  }
  printw("|~~~@~~~~~~~~~@~~~|)\n");

  // Dibujar el camión de NATURA
  for (int i = 0; i < pos_natura; i++) {
    printw(" "); // Espacios para mover el camión a la derecha
  }
  printw("_______________  \n");
  for (int i = 0; i < pos_natura; i++) {
    printw(" ");
  }
  printw("|__|__|__|__|__|___ \n");
  for (int i = 0; i < pos_natura; i++) {
    printw(" ");
  }
  printw("|    NATURA        |\n");
  for (int i = 0; i < pos_natura; i++) {
    printw(" ");
  }
  printw("|~~~@~~~~~~~~~@~~~|)\n");
}

int main() {
  srand(time(NULL)); // Inicializa la semilla aleatoria

  // Inicia ncurses
  initscr();
  noecho();
  cbreak();
  curs_set(0); // No mostrar el cursor

  int pos_uabc = 0;
  int pos_natura = 0;

  // Empezar la carrera
  printw("¡Comienza la carrera de camiones!\n");
  printw("Camion UABC vs. Camion NATURA\n");
  printw("Meta en %d pasos\n\n", META);

  // Ciclo de la carrera
  while (pos_uabc < META && pos_natura < META) {
    clear(); // Limpia la pantalla

    printw("Camion UABC vs. Camion NATURA\n");
    dibujar_carrera(pos_uabc, pos_natura);

    // Mover camiones
    pos_uabc += mover_camion();
    pos_natura += mover_camion();

    // Hacer que se vea el avance
    usleep(500000); // 0.5 segundos de espera
    refresh();      // Refresca la pantalla
  }

  // Fin de la carrera
  clear();
  if (pos_uabc >= META) {
    printw("¡El camion %s gano!\n", CAMION_UABC);
  } else {
    printw("¡El camion %s gano con 3 muertos pero gano!\n", CAMION_NATURA);
  }
  refresh();
  getch(); // Espera una tecla para salir

  endwin(); // Termina ncurses
  return 0;
}
