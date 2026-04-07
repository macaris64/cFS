/************************************************************************
 * ai_app — tensor helpers (inference / reference math)
 ************************************************************************/
#ifndef AI_APP_TENSOR_H
#define AI_APP_TENSOR_H

#include <stddef.h>

void AI_APP_RmsNorm(const double *x, double *out, size_t n);

void AI_APP_Softmax(const double *logits, double *probs, size_t n);

/*
 * Row-major W: out[i] = sum_j W[i * n_in + j] * x[j]
 */
void AI_APP_Linear(const double *w, const double *x, double *out, size_t n_out, size_t n_in);

#endif /* AI_APP_TENSOR_H */
