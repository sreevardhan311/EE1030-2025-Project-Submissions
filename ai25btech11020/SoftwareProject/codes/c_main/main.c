#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "..\c_libs\pgm_io.h"
#include "..\c_libs\svd_utils.h"

int main(void) {
    // taking input of images
    const char *input_files[] = {
        "../../Figs/input/inputimage_1.pgm",
        "../../Figs/input/inputimage_2.pgm",
        "../../Figs/input/inputimage_3.pgm"
    };
    const int num_images = 3;

    // taking k value as txt file i.e. input/k.txt
    int k_values[10];
    int num_k = 0;

    // opening the file
    FILE *kf = fopen("../../Figs/input/k.txt", "r");
    if (!kf) {
        fprintf(stderr, "Error: Cannot open ../../Figs/input/k.txt\n");
        return 1;
    }
    while (num_k < 10 && fscanf(kf, "%d", &k_values[num_k]) == 1) num_k++;
    fclose(kf);
    if (num_k == 0) {
        fprintf(stderr, "Error: No valid k values in ../../Figs/input/k.txt\n");
        return 1;
    }

    // Creating a Frobenius error table file as a txt file in tables

    FILE *table = fopen("../../tables/percentage_error_table.txt", "w");
    if (!table) {
        fprintf(stderr, "Error: Cannot write ../../tables/percentage_error_table.txt\n");
        return 1;
    }

    fprintf(table, "------------------------------------------------------------\n");
    fprintf(table, "|   Image   |   Rank (k)   |   Percentage Error (%%)       |\n");
    fprintf(table, "------------------------------------------------------------\n");

    srand(0); // setting seed (this will produce the same sequence of values every time)

    for (int img_count = 0; img_count < num_images; img_count++) {
        printf("\n=== Processing Image %d: %s ===\n", img_count + 1, input_files[img_count]);

        int m, n;
        float *src = read_pgm(input_files[img_count], &m, &n);
        if (!src) {
            fprintf(stderr, "Error: Cannot read %s\n", input_files[img_count]);
            continue;
        }
        size_t count = (size_t)m * n;

        // Precompute ||A||_F
        double normA = 0.0;
        for (size_t i = 0; i < count; i++) normA += (double)src[i] * src[i];
        normA = sqrt(normA);
        if (normA < 1e-12) normA = 1.0; // to avoid divide by zero

        // Looping over k values
        for (int k_count = 0; k_count < num_k; k_count++) {
            int k = k_values[k_count];
            printf("\n--- Rank k = %d ---\n", k);

            // Allocate memory for the matrices
            float *A_res = (float*)malloc(count * sizeof(float));
            float *A_app = (float*)calloc(count, sizeof(float));
            for (size_t i = 0; i < count; i++) A_res[i] = src[i];

            const int bmax = 16;
            float *V  = (float*)malloc((size_t)n * bmax * sizeof(float));
            float *Y  = (float*)malloc((size_t)m * bmax * sizeof(float));
            float *Z  = (float*)malloc((size_t)n * bmax * sizeof(float));
            float *PrevV = (float*)calloc((size_t)n * k, sizeof(float));

            int prev_cols = 0, prev_ld = k;
            int remaining = k, blockcount = 0;

            // Block SVD
            while (remaining > 0) {
                int curr_b = remaining < bmax ? remaining : bmax;
                printf("Block %d: extracting %d vectors (remaining %d)\n",
                       blockcount + 1, curr_b, remaining);
                svd_block_step(A_res, m, n, curr_b, A_app, V, Y, Z,
                               100, 1e-5f, PrevV, prev_cols, prev_ld);

                // Coping current V into PrevV
                for (int j = 0; j < curr_b; j++)
                    for (int i = 0; i < n; i++)
                        PrevV[(size_t)i * prev_ld + (prev_cols + j)] = V[(size_t)i * bmax + j];

                prev_cols += curr_b;
                remaining -= curr_b;
                blockcount++;
            }

            //Calculating percentage error using RAW A_app (before scaling)
            double diff_norm = 0.0;
            for (size_t i = 0; i < count; i++) {
                double a = (double)src[i];
                double ak = (double)A_app[i];
                double d = a - ak;
                diff_norm += d * d;
            }
            diff_norm = sqrt(diff_norm);
            double percent_error = (diff_norm / normA) * 100.0;

            printf("Percentage error (k=%d): %.4f%%\n", k, percent_error);
            fprintf(table, "|  image%-4d | %8d   | %25.4f |\n", img_count + 1, k, percent_error);

            //Creating a scaled copy for saving
            float *A_vis = (float*)malloc(count * sizeof(float));
            float minv = A_app[0], maxv = A_app[0];
            for (size_t i = 1; i < count; i++) {
                if (A_app[i] < minv) minv = A_app[i];
                if (A_app[i] > maxv) maxv = A_app[i];
            }
            double range = (double)(maxv - minv);
            if (range < 1e-6) range = 1.0;
            for (size_t i = 0; i < count; i++) {
                double v = ((double)A_app[i] - minv) / range;
                A_vis[i] = (float)(255.0 * v);
            }

            // Write image
            char outname[256];
            sprintf(outname, "../../Figs/output/imagee%d_k%d.pgm", img_count + 1, k);
            write_pgm(outname, A_vis, m, n);
            printf("Saved %s\n", outname);

            //Free memory
            free(A_vis);
            free(A_res); free(A_app);
            free(V); free(Y); free(Z); free(PrevV);
        }

        free(src);
    }

    fprintf(table, "------------------------------------------------------------\n");
    fclose(table);

    printf("\n All images processed successfully!\n");
    printf(" Percentage error table saved at ../../tables/percentage_error_table.txt\n");

    fflush(stdout);
    return 0;
}