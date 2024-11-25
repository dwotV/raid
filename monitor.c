#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
#define BYTE_ESPERADO 0xFF
#define POSICION_ESPERADA 100

int verificarModificacionEspecifica(const char *archivo) {
    FILE *f = fopen(archivo, "rb");
    if (f == NULL) {
        perror("Error al abrir el archivo para verificación");
        return 0;
    }
 
    fseek(f, POSICION_ESPERADA, SEEK_SET);
    unsigned char byteLeido = 0;
    fread(&byteLeido, sizeof(unsigned char), 1, f);
    fclose(f);
 
    return byteLeido == BYTE_ESPERADO;
}

// Inicializar inotify y agregar los archivos a monitorear
int inicializarInotify(int *watch1, int *watch2, int fd_inotify, const char *archivo1, const char *archivo2) {
    *watch1 = inotify_add_watch(fd_inotify, archivo1, IN_MODIFY | IN_DELETE_SELF);
    *watch2 = inotify_add_watch(fd_inotify, archivo2, IN_MODIFY | IN_DELETE_SELF);
    if (*watch1 < 0 || *watch2 < 0) {
        perror("Error al agregar archivos a inotify");
        return -1;
    }
    printf("Monitoreando cambios en: %s y %s...\n", archivo1, archivo2);
    return 0;
}

void escucharEventos(int fd_inotify, int *watch1, int *watch2, const char *archivo1, const char *archivo2, const char *archivoParidad) {
    char buffer[EVENT_BUF_LEN];
    char comando[256]; 

    while (1) {
        int length = read(fd_inotify, buffer, EVENT_BUF_LEN);
        if (length < 0) {
            perror("Error en la lectura de eventos");
            break;
        }

        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];

            if (event->mask & IN_DELETE_SELF) {
                if (event->wd == *watch1) {
                    printf("El archivo '%s' ha sido eliminado. Intentando recuperar...\n", archivo1);
                    snprintf(comando, sizeof(comando), "./raid recover %s %s %s", archivo2, archivoParidad, archivo1);
                    if (system(comando) == 0) {
                        printf("Archivo '%s' recuperado con éxito.\n", archivo1);
                        *watch1 = inotify_add_watch(fd_inotify, archivo1, IN_MODIFY | IN_DELETE_SELF);
                    } else {
                        fprintf(stderr, "Error al recuperar el archivo '%s'.\n", archivo1);
                    }
                } else if (event->wd == *watch2) {
                    printf("El archivo '%s' ha sido eliminado. Intentando recuperar...\n", archivo2);
                    snprintf(comando, sizeof(comando), "./raid recover %s %s %s", archivo1, archivoParidad, archivo2);
                    if (system(comando) == 0) {
                        printf("Archivo '%s' recuperado con éxito.\n", archivo2);
                        *watch2 = inotify_add_watch(fd_inotify, archivo2, IN_MODIFY | IN_DELETE_SELF);
                    } else {
                        fprintf(stderr, "Error al recuperar el archivo '%s'.\n", archivo2);
                    }
                }
            } else if (event->mask & IN_MODIFY) {
                if (event->wd == *watch1) {
                    printf("Cambio detectado en '%s'. Verificando contenido...\n", archivo1);
                    if (verificarModificacionEspecifica(archivo1)) {
                        printf("Se detectó un ERROR en '%s'. Recuperando archivo...\n", archivo1);
                        snprintf(comando, sizeof(comando), "./raid recover %s %s %s", archivo2, archivoParidad, archivo1);
                        if (system(comando) != 0) {
                            fprintf(stderr, "Error al ejecutar el comando de recuperación para '%s'.\n", archivo1);
                        }
                    } else {
                        printf("Modificación en '%s'. Actualizando archivo de paridad...\n", archivo1);
                        snprintf(comando, sizeof(comando), "./raid create %s %s", archivo1, archivo2);
                        if (system(comando) != 0) {
                            fprintf(stderr, "Error al crear el archivo de paridad.\n");
                        }
                    }
                } else if (event->wd == *watch2) {
                    printf("Cambio detectado en '%s'. Verificando contenido...\n", archivo2);
                    if (verificarModificacionEspecifica(archivo2)) {
                        printf("Se detectó un ERROR en '%s'. Recuperando archivo...\n", archivo2);
                        snprintf(comando, sizeof(comando), "./raid recover %s %s %s", archivo1, archivoParidad, archivo2);
                        if (system(comando) != 0) {
                            fprintf(stderr, "Error al ejecutar el comando de recuperación para '%s'.\n", archivo2);
                        }
                    } else {
                        printf("Modificación en '%s'. Actualizando archivo de paridad...\n", archivo2);
                        snprintf(comando, sizeof(comando), "./raid create %s %s", archivo1, archivo2);
                        if (system(comando) != 0) {
                            fprintf(stderr, "Error al crear el archivo de paridad.\n");
                        }
                    }
                }
            }
            i += EVENT_SIZE + (event->len > 0 ? event->len : 0);
        }
    }
}

void limpiarInotify(int fd_inotify, int watch1, int watch2) {
    inotify_rm_watch(fd_inotify, watch1);
    inotify_rm_watch(fd_inotify, watch2);
    close(fd_inotify);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <archivo1> <archivo2> <archivo_paridad>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd_inotify, watch1, watch2;
    const char *archivo1 = argv[1];
    const char *archivo2 = argv[2];
    const char *archivoParidad = argv[3];

    fd_inotify = inotify_init();
    if (fd_inotify < 0) {
        perror("Error al inicializar inotify");
        exit(EXIT_FAILURE);
    }

    if (inicializarInotify(&watch1, &watch2, fd_inotify, archivo1, archivo2) < 0) {
        close(fd_inotify);
        exit(EXIT_FAILURE);
    }

    escucharEventos(fd_inotify, &watch1, &watch2, archivo1, archivo2, archivoParidad);

    limpiarInotify(fd_inotify, watch1, watch2);
    return 0;
}
