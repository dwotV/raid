#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 4096  

void crearParity(FILE *f1, FILE *f2, FILE *parity) {
    unsigned char byte1, byte2, parityByte;
    size_t read1, read2;

    while (1) {
        read1 = fread(&byte1, sizeof(unsigned char), 1, f1);
        read2 = fread(&byte2, sizeof(unsigned char), 1, f2);

        if (read1 == 0 && read2 == 0) {
            break;
        }

        if (read1 != read2) {
            fprintf(stderr, "Error: Los archivos tienen tamaños diferentes.\n");
            exit(EXIT_FAILURE);
        }

        if (read1 == 1 && read2 == 1) {
            parityByte = byte1 ^ byte2;
            fwrite(&parityByte, sizeof(unsigned char), 1, parity);
        }
    }
}

void recuperarArchivo(FILE *existing_file, FILE *parity, FILE *recovered_file) {
    unsigned char byteExistente, parityByte, byteRecuperado;
    size_t readExistente, readParity;

    while (1) {
        readExistente = fread(&byteExistente, sizeof(unsigned char), 1, existing_file);
        readParity = fread(&parityByte, sizeof(unsigned char), 1, parity);

        if (readExistente == 0 && readParity == 0) {
            break;
        }

        if (readExistente != readParity) {
            fprintf(stderr, "Error: Los archivos tienen tamaños diferentes.\n");
            exit(EXIT_FAILURE);
        }

        if (readExistente == 1 && readParity == 1) {
            byteRecuperado = byteExistente ^ parityByte;
            fwrite(&byteRecuperado, sizeof(unsigned char), 1, recovered_file);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso:\n");
        fprintf(stderr, "  Para crear paridad:\n");
        fprintf(stderr, "    %s create <archivo1.txt> <archivo2.txt>\n", argv[0]);
        fprintf(stderr, "  Para recuperar un archivo:\n");
        fprintf(stderr, "    %s recover <archivo_existente.txt> <parity.bin> <archivo_recuperado.txt>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "create") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Uso para crear paridad:\n");
            fprintf(stderr, "  %s create <archivo1.txt> <archivo2.txt>\n", argv[0]);
            return EXIT_FAILURE;
        }

        const char *file1 = argv[2];
        const char *file2 = argv[3];
        const char *nombreArchivoParity = "parity.bin";

        FILE *f1 = fopen(file1, "rb");
        if (f1 == NULL) {
            perror("Error al abrir el primer archivo de entrada");
            return EXIT_FAILURE;
        }

        FILE *f2 = fopen(file2, "rb");
        if (f2 == NULL) {
            perror("Error al abrir el segundo archivo de entrada");
            fclose(f1);
            return EXIT_FAILURE;
        }

        FILE *parity = fopen(nombreArchivoParity, "wb");
        if (parity == NULL) {
            perror("Error al crear el archivo de paridad");
            fclose(f1);
            fclose(f2);
            return EXIT_FAILURE;
        }

        crearParity(f1, f2, parity);

        printf("Archivo de paridad '%s' creado con éxito.\n", nombreArchivoParity);

        fclose(f1);
        fclose(f2);
        fclose(parity);

    } else if (strcmp(argv[1], "recover") == 0) {
        if (argc != 5) {
            fprintf(stderr, "Uso para recuperar archivo:\n");
            fprintf(stderr, "  %s recover <archivo_existente.txt> <parity.bin> <archivo_recuperado.txt>\n", argv[0]);
            return EXIT_FAILURE;
        }

        const char *nombreArchivoExistente = argv[2];
        const char *nombreArchivoParity = argv[3];
        const char *nombreArchivoRecuperado = argv[4];

        FILE *archivoExistente = fopen(nombreArchivoExistente, "rb");
        if (archivoExistente == NULL) {
            perror("Error al abrir el archivo existente");
            return EXIT_FAILURE;
        }

        FILE *parity = fopen(nombreArchivoParity, "rb");
        if (parity == NULL) {
            perror("Error al abrir el archivo de paridad");
            fclose(archivoExistente);
            return EXIT_FAILURE;
        }

        FILE *archivoRecuperado = fopen(nombreArchivoRecuperado, "wb");
        if (archivoRecuperado == NULL) {
            perror("Error al crear el archivo recuperado");
            fclose(archivoExistente);
            fclose(parity);
            return EXIT_FAILURE;
        }

        recuperarArchivo(archivoExistente, parity, archivoRecuperado);

        printf("Archivo '%s' recuperado con éxito.\n", nombreArchivoRecuperado);

        fclose(archivoExistente);
        fclose(parity);
        fclose(archivoRecuperado);

    } else {
        fprintf(stderr, "Modo no reconocido. Usa 'create' para crear paridad o 'recover' para recuperación.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
