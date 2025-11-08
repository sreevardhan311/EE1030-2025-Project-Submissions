#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "..\c_libs\stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "..\c_libs\stb_image_write.h"



int main(void) {
    // Define input and output file paths
    const char *input_file = "../../Figs/input/greyscale.png";      
    const char *output_file = "../../Figs/input/inputimage_3.pgm"; 
    // output file is .pgm file
    int width, height, channels;
    unsigned char *img = stbi_load(input_file, &width, &height, &channels, 0);
    if (!img) {
        fprintf(stderr, "Error: could not load image '%s'\n", input_file);
        return 1;
    }

    // Allocate memory for grayscale dat using
    unsigned char *gray = (unsigned char *)malloc(width * height);
    if (!gray) {
        fprintf(stderr, "Error: could not allocate memory.\n");
        stbi_image_free(img);
        return 1;
    }

   
    for (int i = 0; i < width * height; i++)
     {
        
            gray[i] = img[i * channels]; 
        
     }

    // Write output PGM file in binary (P5) format
    FILE *f = fopen(output_file, "wb");
    if (!f) {
        fprintf(stderr, "Error: could not open output file '%s'\n", output_file);
        free(gray);
        stbi_image_free(img);
        return 1;
    }

    fprintf(f, "P5\n%d %d\n255\n", width, height);
    fwrite(gray, 1, width * height, f);
    fclose(f);

    printf("Successfully converted '%s' -> '%s'\n", input_file, output_file);

    stbi_image_free(img);
    free(gray);
    return 0;
}
