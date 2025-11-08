#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "..\c_libs\svd_utils.h"

/*Basic Vector Utility Functions*/

/* This function finds the length (norm) of a vector */
float vector_norm(const float *vec, int length) {
    double total = 0.0;
    for (int i = 0; i < length; ++i)
        total += (double)vec[i] * vec[i];
    return (float)sqrt(total);
}

/* This function divides each element by the norm 
   so that total length becomes 1 */
void normalize_vector(float *vec, int length) {
    float norm_val = vector_norm(vec, length);
    if (norm_val < 1e-12f) return;  // to avoid division by zero
    for (int i = 0; i < length; ++i)
        vec[i] /= norm_val;
}

/*Matrix Multiplication Functions*/

/* Multiply matrices like this: Y = A * V
   A is of size m×n, V is n×b, result Y will be m×b */
void matmul_A_times_V(const float *A, const float *V, float *Y, int m, int n, int b) {
    for (int i = 0; i < m; ++i) {
        const float *rowA = A + (size_t)i * n;
        float *rowY = Y + (size_t)i * b;

        // First make the row of Y all zeros
        for (int j = 0; j < b; ++j)
            rowY[j] = 0.0f;

        // Multiply row of A with each column of V
        for (int k = 0; k < n; ++k) {
            float a_val = rowA[k];
            const float *colV = V + (size_t)k * b;
            for (int j = 0; j < b; ++j)
                rowY[j] += a_val * colV[j];
        }
    }
}

/* Multiply matrices like this: Z = Aᵀ * Y
   Here A is m×n, Y is m×b, so result Z will be n×b */
void matmul_AT_times_Y(const float *A, const float *Y, float *Z, int m, int n, int b) {
    for (int j = 0; j < n; ++j) {
        float *colZ = Z + (size_t)j * b;

        // Make this column zero first
        for (int t = 0; t < b; ++t)
            colZ[t] = 0.0f;

        // Multiply A transpose with Y
        for (int i = 0; i < m; ++i) {
            float a_val = A[(size_t)i * n + j];
            const float *rowY = Y + (size_t)i * b;
            for (int t = 0; t < b; ++t)
                colZ[t] += a_val * rowY[t];
        }
    }
}

/*QR Orthogonalization using Gram-Schmidt*/

/* This makes all columns of Z perpendicular (orthogonal) 
   and also gives them length = 1 */
void qr_modified_gram_schmidt(float *Z, int n, int b, int ld) {
    for (int j = 0; j < b; ++j) {
        // Subtract projection on all earlier columns
        for (int k = 0; k < j; ++k) {
            double dot_sum = 0.0;
            for (int i = 0; i < n; ++i)
                dot_sum += (double)Z[(size_t)i * ld + k] * Z[(size_t)i * ld + j];

            float proj = (float)dot_sum;
            for (int i = 0; i < n; ++i)
                Z[(size_t)i * ld + j] -= proj * Z[(size_t)i * ld + k];
        }

        // Now divide by its own length to make length 1
        double sq_sum = 0.0;
        for (int i = 0; i < n; ++i) {
            double temp = Z[(size_t)i * ld + j];
            sq_sum += temp * temp;
        }
        sq_sum = sqrt(sq_sum);

        float inv = (sq_sum == 0.0) ? 0.0f : (float)(1.0 / sq_sum);
        for (int i = 0; i < n; ++i)
            Z[(size_t)i * ld + j] *= inv;
    }
}

/*Make new V vectors orthogonal to old ones*/

/* This removes any part of current V that lies along previous V
   so that all vectors stay independent */
void orthogonalize_against_prev(float *V, int n, int b,
                                float *PrevV, int prev_cols, int ld_prev) {
    if (prev_cols <= 0) return;

    for (int j = 0; j < b; ++j) {
        for (int k = 0; k < prev_cols; ++k) {
            double sum = 0.0;
            for (int i = 0; i < n; ++i)
                sum += (double)PrevV[(size_t)i * ld_prev + k] * V[(size_t)i * b + j];

            float proj = (float)sum;
            for (int i = 0; i < n; ++i)
                V[(size_t)i * b + j] -= proj * PrevV[(size_t)i * ld_prev + k];
        }
        // Finally normalize this column
        normalize_vector(&V[j], n);
    }
}

/* Main Block Power Iteration for SVD */

/* This function performs one block step of SVD
   It updates the approximation and removes that part from residual */
void svd_block_step(float *A_residual, int m, int n, int b,
                    float *A_approx,
                    float *tempV, float *tempY, float *tempZ,
                    int max_iter, float tol,
                    const float *PrevV, int prev_cols, int ld_prev)
{
    // First we give V some small random-like values
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < b; ++j)
            tempV[(size_t)i * b + j] = 1.0f + 1e-3f * (float)(j + (i % 7));

    // Make these V vectors orthogonal to earlier found ones
    orthogonalize_against_prev(tempV, n, b, (float*)PrevV, prev_cols, ld_prev);

    // Temporary copy of V to check when it stops changing
    float *V_old = (float*)malloc((size_t)n * b * sizeof(float));
    if (!V_old) return;

    for (int iter = 0; iter < max_iter; ++iter) {
        memcpy(V_old, tempV, n * b * sizeof(float));

        // Step 1: Y = A * V
        matmul_A_times_V(A_residual, tempV, tempY, m, n, b);

        // Step 2: Z = Aᵀ * Y
        matmul_AT_times_Y(A_residual, tempY, tempZ, m, n, b);

        // Step 3: Make Z orthogonal to get new V
        qr_modified_gram_schmidt(tempZ, n, b, b);
        memcpy(tempV, tempZ, n * b * sizeof(float));

        // Step 4: Check how much V changed from last time
        double diff_sum = 0.0;
        for (int i = 0; i < n * b; ++i) {
            double d = (double)tempV[i] - (double)V_old[i];
            diff_sum += d * d;
        }
        if (sqrt(diff_sum) < tol)
            break; // if change is small, we can stop
    }

    free(V_old);

    // Now make the low-rank update in A_approx and remove that from A_residual
    matmul_A_times_V(A_residual, tempV, tempY, m, n, b);

    for (int j = 0; j < b; ++j) {
        double sigma_sqrs = 0.0;

        // Find singular value (sigma)
        for (int i = 0; i < m; ++i) {
            double val = tempY[(size_t)i * b + j];
            sigma_sqrs += val * val;
        }

        double sigma = sqrt(sigma_sqrs);
        if (sigma < 1e-12)
            continue;

        // Update both A_approx and A_residual
        for (int i = 0; i < m; ++i) {
            float u_i = tempY[(size_t)i * b + j] / (float)sigma;
            float su_i = (float)sigma * u_i;

            for (int k = 0; k < n; ++k) {
                float v_kj = tempV[(size_t)k * b + j];
                float up = su_i * v_kj;
                A_approx[(size_t)i * n + k] += up;   // adding this part to the approximation
                A_residual[(size_t)i * n + k] -= up; // removing same part from residual
            }
        }
    }
}
