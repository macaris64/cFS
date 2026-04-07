/************************************************************************
 * ai_app tensor ops (matches microgpt numerics)
 ************************************************************************/

#include "ai_app_tensor.h"

#include <math.h>

void AI_APP_RmsNorm(const double *x, double *out, size_t n)
{
    size_t i;
    double ms;
    double scale;
    if (x == NULL || out == NULL || n == 0)
    {
        return;
    }
    ms = 0.0;
    for (i = 0; i < n; i++)
    {
        ms += x[i] * x[i];
    }
    ms /= (double)n;
    scale = 1.0 / sqrt(ms + 1e-5);
    for (i = 0; i < n; i++)
    {
        out[i] = x[i] * scale;
    }
}

void AI_APP_Softmax(const double *logits, double *probs, size_t n)
{
    size_t i;
    double m;
    double sum;
    if (logits == NULL || probs == NULL || n == 0)
    {
        return;
    }
    m = logits[0];
    for (i = 1; i < n; i++)
    {
        if (logits[i] > m)
        {
            m = logits[i];
        }
    }
    sum = 0.0;
    for (i = 0; i < n; i++)
    {
        probs[i] = exp(logits[i] - m);
        sum += probs[i];
    }
    for (i = 0; i < n; i++)
    {
        probs[i] /= sum;
    }
}

void AI_APP_Linear(const double *w, const double *x, double *out, size_t n_out, size_t n_in)
{
    size_t i;
    size_t j;
    if (w == NULL || x == NULL || out == NULL)
    {
        return;
    }
    for (i = 0; i < n_out; i++)
    {
        double acc = 0.0;
        for (j = 0; j < n_in; j++)
        {
            acc += w[i * n_in + j] * x[j];
        }
        out[i] = acc;
    }
}
