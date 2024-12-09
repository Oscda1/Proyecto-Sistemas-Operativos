#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINEA 1024
#define AZUL "\033[36m"
#define BLANCO "\033[0m"

void mostrar_ayuda() {
  printf("Comandos disponibles:\n");
  printf("  estado        - Ejecuta una verificacion del estado de computo\n");
  printf("  ppm           - Test de palabras por minuto\n");
  printf("  crear         - Crea un archivo\n");
  printf("  leer          - Lee un archivo\n");
  printf("  editar        - Edita un archivo\n");
  printf("  agenda        - Agenda de tareas\n");
  printf("  help          - Muestra esta ayuda\n");
  printf("  exit          - Salir del shell\n");
  printf("  buscador      - Buscador de comandos ?\n");
  printf("  camiones      - Carrera de camiones\n");
  printf("  boveda        - Encriptacion/Desencriptacion de archivos\n");
}

void ejecutar_comando(char *comando) {
  char *argumentos[MAX_LINEA];
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

void crear(char **args) {
  int fd = open(args[1], O_CREAT | O_WRONLY, 0644);
  if (fd == -1) {
    perror("Error al crear el archivo");
    return;
  }
  close(fd);
  printf("Archivo %s creado exitosamente.\n", args[1]);
}

void leer(char **args) {
  int fd = open(args[1], O_RDONLY);
  if (fd == -1) {
    perror("Error al abrir el archivo");
    return;
  }
  char buffer[1024];
  ssize_t bytes_leidos;
  while ((bytes_leidos = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
    buffer[bytes_leidos] = '\0';
    printf("%s", buffer);
  }
  close(fd);
}

void editar(char **args) {
  int fd = open(args[1], O_WRONLY | O_APPEND);
  if (fd == -1) {
    perror("Error al abrir el archivo");
    return;
  }
  char buffer[1024];
  printf("Ingrese el texto a agregar al archivo:\n");
  if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
    perror("Error al leer la entrada");
    close(fd);
    return;
  }
  if (write(fd, buffer, strlen(buffer)) == -1) {
    perror("Error al escribir en el archivo");
  }
  close(fd);
  printf("Archivo %s editado exitosamente.\n", args[1]);
}

void ejecutar_cd(char *comando) {
  char *directorio = strtok(comando + 3, " \n");

  if (directorio == NULL) {
    directorio = getenv("HOME");
    if (directorio == NULL) {
      fprintf(stderr, "Error: no se pudo determinar el directorio HOME.\n");
      return;
    }
  }

  if (chdir(directorio) != 0) {
    perror("Error al cambiar de directorio");
  } else {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      printf("Directorio actual: %s\n", cwd);
    } else {
      perror("Error al obtener el directorio actual");
    }
  }
}

void ejecutar_redireccion(char *comando) {
  char *comando_base;
  char *archivo;
  int concatenar = 0;

  size_t len = strlen(comando);
  if (len > 0 && comando[len - 1] == '\n') {
    comando[len - 1] = '\0';
  }

  if ((archivo = strstr(comando, ">>")) != NULL) {
    concatenar = 1;
  } else if ((archivo = strchr(comando, '>')) != NULL) {
    concatenar = 0;
  } else {
    ejecutar_comando(comando);
    return;
  }

  *archivo = '\0';
  archivo += concatenar ? 2 : 1;
  while (*archivo == ' ')
    archivo++;

  if (*archivo == '\0') {
    fprintf(stderr, "Error: no se especifico archivo para redireccion.\n");
    return;
  }

  int fd = open(archivo, O_WRONLY | O_CREAT | (concatenar ? O_APPEND : O_TRUNC),
                0644);
  if (fd == -1) {
    perror("Error al abrir el archivo");
    return;
  }

  comando_base = strtok(comando, "\n");
  if (fork() == 0) {
    dup2(fd, STDOUT_FILENO);
    close(fd);
    ejecutar_comando(comando_base);
    exit(EXIT_FAILURE);
  } else {
    close(fd);
    wait(NULL);
  }
}

void ejecutar_pipe(char *comando) {
  char *comandos[2];
  comandos[0] = strtok(comando, "|");
  comandos[1] = strtok(NULL, "|");

  int fd[2];
  if (pipe(fd) == -1) {
    perror("Error al crear el pipe");
    return;
  }

  if (fork() == 0) {
    dup2(fd[1], STDOUT_FILENO);
    close(fd[0]);
    close(fd[1]);
    ejecutar_comando(comandos[0]);
    exit(EXIT_FAILURE);
  }

  if (fork() == 0) {
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]);
    close(fd[1]);
    ejecutar_comando(comandos[1]);
    exit(EXIT_FAILURE);
  }

  close(fd[0]);
  close(fd[1]);
  wait(NULL);
  wait(NULL);
}

#define PATH_MAX 256

int main() {
  char comando[MAX_LINEA];
  char cwd[PATH_MAX];
      if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return 0;
    }

    const char *current_path = getenv("PATH");
    if (current_path == NULL) {
        fprintf(stderr, "Error: No se pudo obtener el PATH actual.\n");
        return 0;
    }

    // Crear un nuevo PATH con el directorio actual
    size_t new_path_size = strlen(current_path) + strlen(cwd) + 2; // 1 para ':' y 1 para '\0'
    char *new_path = malloc(new_path_size);
    if (new_path == NULL) {
        perror("malloc");
        return 0;
    }

    snprintf(new_path, new_path_size, "%s:%s", current_path, cwd);

    // Establecer la nueva variable de entorno PATH
    if (setenv("PATH", new_path, 1) != 0) {
        perror("setenv");
        free(new_path);
        return 0;
    }
    free(new_path);
  // Mensaje de bienvenida
  printf("Bienvenido al shell de los pollos hermanos!\n");
  printf("Recuerda verificar si tienes tareas pendientes!.\n");
  printf("Escribe 'help' para ver los comandos disponibles.\n");

  while (1) {
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      printf(AZUL "%s" BLANCO " $ ", cwd);
    } else {
      perror("Error al obtener el directorio actual");
      return 1;
    }
    if (fgets(comando, MAX_LINEA, stdin) == NULL) {
      break;
    }

    if (strcmp(comando, "exit\n") == 0) {
      // Mensaje de salida
      printf("Saliendo del shell. ¡Hasta luego!\n");
      break;
    } else if (strcmp(comando, "help\n") == 0) {
      mostrar_ayuda();
    } else if (strncmp(comando, "crear", 5) == 0) {
      char *args[] = {strtok(comando, " \n"), strtok(NULL, " \n"), NULL};
      crear(args);
    } else if (strncmp(comando, "leer", 4) == 0) {
      char *args[] = {strtok(comando, " \n"), strtok(NULL, " \n"), NULL};
      leer(args);
    } else if (strncmp(comando, "editar", 6) == 0) {
      char *args[] = {strtok(comando, " \n"), strtok(NULL, " \n"), NULL};
      editar(args);
    } else if (strncmp(comando, "cd", 2) == 0) {
      ejecutar_cd(comando);
    } else if (strchr(comando, '|') != NULL) {
      ejecutar_pipe(comando);
    } else if (strchr(comando, '>') != NULL) {
      ejecutar_redireccion(comando);
    } else {
      ejecutar_comando(comando);
    }
  }

  return 0;
}
