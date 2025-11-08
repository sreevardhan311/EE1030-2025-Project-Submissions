#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../c_libs/stb_image_write.h"

//  Function to read a binary (P5) PGM image 
unsigned char* read_pgm(const char *filename, int *width, int *height) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, " Cannot open file: %s\n", filename);
        return NULL;
    }

    char identifier[3];
    if (!fgets(identifier, sizeof(identifier), fp)) {
        fprintf(stderr, "Cannot read identifier number: %s\n", filename);
        fclose(fp);
        return NULL;
    }

    if (strncmp(identifier, "P5", 2) != 0) {
        fprintf(stderr, " Unsupported PGM format (must be P5): %s\n", filename);
        fclose(fp);
        return NULL;
    }

    // Skip comments
    int c;
    do {
        c = fgetc(fp);
        if (c == '#') while (fgetc(fp) != '\n');
    } while (c == '#' || c == '\n' || c == '\r' || c == ' ');

    ungetc(c, fp);

    int maxval;
    if (fscanf(fp, "%d %d %d", width, height, &maxval) != 3) {
        fprintf(stderr, " Invalid PGM header: %s\n", filename);
        fclose(fp);
        return NULL;
    }

    fgetc(fp); // skip one whitespace

    int size = (*width) * (*height);
    unsigned char *data = (unsigned char*)malloc(size);
    if (!data) {
        fprintf(stderr, " Memory allocation failed\n");
        fclose(fp);
        return NULL;
    }

    if (fread(data, 1, size, fp) != size) {
        fprintf(stderr, " Error reading image data: %s\n", filename);
        free(data);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return data;
}

// ---------- Main Program ----------
int main() {
    const char *base_path = "../../Figs/output/";

    // list of my 12 PGM files
    const char *pgm_files[] = {
        "imagee1_k5.pgm",
        "imagee1_k10.pgm",
        "imagee1_k15.pgm",
        "imagee1_k20.pgm",
        "imagee2_k5.pgm",
        "imagee2_k10.pgm",
        "imagee2_k15.pgm",
        "imagee2_k20.pgm",
        "imagee3_k5.pgm",
        "imagee3_k10.pgm",
        "imagee3_k15.pgm",
        "imagee3_k20.pgm"
    };

    int num_files = sizeof(pgm_files) / sizeof(pgm_files[0]);

    for (int i = 0; i < num_files; i++) {
        char input_path[512];
        snprintf(input_path, sizeof(input_path), "%s%s", base_path, pgm_files[i]);

        int width, height;
        unsigned char *image = read_pgm(input_path, &width, &height);
        if (!image) continue;

        char output_path[512];
        snprintf(output_path, sizeof(output_path), "%s", input_path);
        char *dot = strrchr(output_path, '.');
        if (dot) strcpy(dot, ".png");
        else strcat(output_path, ".png");

        if (stbi_write_png(output_path, width, height, 1, image, width)) {
            printf(" Converted: %s → %s\n", pgm_files[i], strrchr(output_path, '/') + 1);
        } else {
            fprintf(stderr, " Failed to write: %s\n", output_path);
        }

        free(image);
    }

    printf("\n Conversion complete! PNG files saved in ../../Figs/output/\n");
    return 0;
}
