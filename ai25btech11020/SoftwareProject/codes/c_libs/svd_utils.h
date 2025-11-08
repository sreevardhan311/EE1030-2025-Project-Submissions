#ifndef SVD_UTILS_H
#define SVD_UTILS_H

float norm_vec(const float *x, int n);
void normalize_vec(float *x, int n);
void matmul_A_V(const float *A, const float *V, float *Y, int m, int n, int b);
void matmul_AT_Y(const float *A, const float *Y, float *Z, int m, int n, int b);
void qr_mgs_block(float *Z, int n, int b, int ld);
void orth_against_prev(float *V, int n, int b, float *Prev, int prev_cols, int ldp);
void svd_block_step(float *A_res, int m, int n, int b,
                    float *A_app,
                    float *workV, float *workY, float *workZ,
                    int max_iter, float tol,
                    const float *PrevV, int prev_cols, int prev_ld);

#endif
