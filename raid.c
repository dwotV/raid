#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 4096  

void create_parity(FILE *f1, FILE *f2, FILE *parity) {
    unsigned char byte1, byte2, parity_byte;
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
            parity_byte = byte1 ^ byte2;
            fwrite(&parity_byte, sizeof(unsigned char), 1, parity);
        }
    }
}

void recover_missing_file(FILE *existing_file, FILE *parity, FILE *recovered_file) {
    unsigned char existing_byte, parity_byte, recovered_byte;
    size_t read_existing, read_parity;

    while (1) {
        read_existing = fread(&existing_byte, sizeof(unsigned char), 1, existing_file);
        read_parity = fread(&parity_byte, sizeof(unsigned char), 1, parity);

        if (read_existing == 0 && read_parity == 0) {
            break;
        }

        if (read_existing != read_parity) {
            fprintf(stderr, "Error: Los archivos tienen tamaños diferentes.\n");
            exit(EXIT_FAILURE);
        }

        if (read_existing == 1 && read_parity == 1) {
            recovered_byte = existing_byte ^ parity_byte;
            fwrite(&recovered_byte, sizeof(unsigned char), 1, recovered_file);
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
        const char *parity_filename = "parity.bin";

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

        FILE *parity = fopen(parity_filename, "wb");
        if (parity == NULL) {
            perror("Error al crear el archivo de paridad");
            fclose(f1);
            fclose(f2);
            return EXIT_FAILURE;
        }

        create_parity(f1, f2, parity);

        printf("Archivo de paridad '%s' creado con éxito.\n", parity_filename);

        fclose(f1);
        fclose(f2);
        fclose(parity);

    } else if (strcmp(argv[1], "recover") == 0) {
        if (argc != 5) {
            fprintf(stderr, "Uso para recuperar archivo:\n");
            fprintf(stderr, "  %s recover <archivo_existente.txt> <parity.bin> <archivo_recuperado.txt>\n", argv[0]);
            return EXIT_FAILURE;
        }

        const char *existing_file_name = argv[2];
        const char *parity_file_name = argv[3];
        const char *recovered_file_name = argv[4];

        FILE *existing_file = fopen(existing_file_name, "rb");
        if (existing_file == NULL) {
            perror("Error al abrir el archivo existente");
            return EXIT_FAILURE;
        }

        FILE *parity = fopen(parity_file_name, "rb");
        if (parity == NULL) {
            perror("Error al abrir el archivo de paridad");
            fclose(existing_file);
            return EXIT_FAILURE;
        }

        FILE *recovered_file = fopen(recovered_file_name, "wb");
        if (recovered_file == NULL) {
            perror("Error al crear el archivo recuperado");
            fclose(existing_file);
            fclose(parity);
            return EXIT_FAILURE;
        }

        recover_missing_file(existing_file, parity, recovered_file);

        printf("Archivo '%s' recuperado con éxito.\n", recovered_file_name);

        fclose(existing_file);
        fclose(parity);
        fclose(recovered_file);

    } else {
        fprintf(stderr, "Modo no reconocido. Usa 'create' para crear paridad o 'recover' para recuperación.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
