/************************************************************************
 * ai_app GPT forward (static KV cache, double precision)
 ************************************************************************/

#include "ai_app_gpt.h"

#include <math.h>
#include <string.h>

#include "ai_app_tensor.h"

static double g_kv_k[AI_APP_GPT_MAX_LAYER][AI_APP_GPT_BLOCK_SIZE][AI_APP_GPT_N_EMBD];
static double g_kv_v[AI_APP_GPT_MAX_LAYER][AI_APP_GPT_BLOCK_SIZE][AI_APP_GPT_N_EMBD];

static double s_x[AI_APP_GPT_N_EMBD];
static double s_xn[AI_APP_GPT_N_EMBD];
static double s_xr[AI_APP_GPT_N_EMBD];
static double s_q[AI_APP_GPT_N_EMBD];
static double s_k[AI_APP_GPT_N_EMBD];
static double s_v[AI_APP_GPT_N_EMBD];
static double s_x_attn[AI_APP_GPT_N_EMBD];
static double s_head_out[AI_APP_GPT_N_EMBD];
static double s_attn_logits[AI_APP_GPT_BLOCK_SIZE];
static double s_attn_w[AI_APP_GPT_BLOCK_SIZE];
static double s_mlp_hid[AI_APP_GPT_MLP_HIDDEN];
static double s_mlp_out[AI_APP_GPT_N_EMBD];

void AI_APP_GptState_Init(AI_APP_GptState *st)
{
    size_t li;
    if (st == NULL)
    {
        return;
    }
    for (li = 0; li < AI_APP_GPT_MAX_LAYER; li++)
    {
        st->kv_len[li] = 0;
    }
}

static void relu_inplace(double *a, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++)
    {
        if (a[i] < 0.0)
        {
            a[i] = 0.0;
        }
    }
}

int AI_APP_GptForwardStep(const AI_APP_GptWeights *weights, AI_APP_GptState *st, uint32_t token_id, uint32_t pos_id,
                          double *logits_out)
{
    size_t n_embd;
    size_t vocab;
    size_t n_head;
    size_t n_layer;
    size_t head_dim;
    double inv_sqrt_head;
    size_t li;
    size_t j;

    if (weights == NULL || st == NULL || logits_out == NULL)
    {
        return -1;
    }

    vocab   = weights->vocab_size;
    n_embd  = weights->n_embd;
    n_head  = weights->n_head;
    n_layer = weights->n_layer;

    if (vocab == 0 || n_embd == 0 || n_head == 0 || n_layer == 0 || n_layer > AI_APP_GPT_MAX_LAYER)
    {
        return -1;
    }
    if ((n_embd % n_head) != 0)
    {
        return -1;
    }
    head_dim = n_embd / n_head;
    if (token_id >= vocab || pos_id >= weights->block_size)
    {
        return -1;
    }

    if (pos_id == 0)
    {
        AI_APP_GptState_Init(st);
    }

    inv_sqrt_head = 1.0 / sqrt((double)head_dim);

    {
        const double *te = weights->wte + (size_t)token_id * n_embd;
        const double *pe = weights->wpe + (size_t)pos_id * n_embd;
        for (j = 0; j < n_embd; j++)
        {
            s_x[j] = te[j] + pe[j];
        }
    }
    AI_APP_RmsNorm(s_x, s_x, n_embd);

    for (li = 0; li < n_layer; li++)
    {
        const AI_APP_GptLayerWeights *lw = &weights->layers[li];
        size_t                        h;
        size_t                        t;
        size_t                        hs;
        size_t                        jj;

        memcpy(s_xr, s_x, n_embd * sizeof(double));
        AI_APP_RmsNorm(s_x, s_xn, n_embd);

        AI_APP_Linear(lw->attn_wq, s_xn, s_q, n_embd, n_embd);
        AI_APP_Linear(lw->attn_wk, s_xn, s_k, n_embd, n_embd);
        AI_APP_Linear(lw->attn_wv, s_xn, s_v, n_embd, n_embd);

        memcpy(g_kv_k[li][pos_id], s_k, n_embd * sizeof(double));
        memcpy(g_kv_v[li][pos_id], s_v, n_embd * sizeof(double));
        st->kv_len[li] = (int32_t)(pos_id + 1u);

        memset(s_head_out, 0, n_embd * sizeof(double));

        for (h = 0; h < n_head; h++)
        {
            hs = h * head_dim;
            for (t = 0; t < (size_t)st->kv_len[li]; t++)
            {
                double dot = 0.0;
                for (jj = 0; jj < head_dim; jj++)
                {
                    dot += s_q[hs + jj] * g_kv_k[li][t][hs + jj];
                }
                s_attn_logits[t] = dot * inv_sqrt_head;
            }
            AI_APP_Softmax(s_attn_logits, s_attn_w, (size_t)st->kv_len[li]);

            for (jj = 0; jj < head_dim; jj++)
            {
                double acc = 0.0;
                for (t = 0; t < (size_t)st->kv_len[li]; t++)
                {
                    acc += s_attn_w[t] * g_kv_v[li][t][hs + jj];
                }
                s_head_out[hs + jj] = acc;
            }
        }

        AI_APP_Linear(lw->attn_wo, s_head_out, s_x_attn, n_embd, n_embd);
        for (j = 0; j < n_embd; j++)
        {
            s_x[j] = s_x_attn[j] + s_xr[j];
        }

        memcpy(s_xr, s_x, n_embd * sizeof(double));
        AI_APP_RmsNorm(s_x, s_xn, n_embd);
        AI_APP_Linear(lw->mlp_fc1, s_xn, s_mlp_hid, AI_APP_GPT_MLP_HIDDEN, n_embd);
        relu_inplace(s_mlp_hid, AI_APP_GPT_MLP_HIDDEN);
        AI_APP_Linear(lw->mlp_fc2, s_mlp_hid, s_mlp_out, n_embd, AI_APP_GPT_MLP_HIDDEN);
        for (j = 0; j < n_embd; j++)
        {
            s_x[j] = s_mlp_out[j] + s_xr[j];
        }
    }

    AI_APP_Linear(weights->lm_head, s_x, logits_out, vocab, n_embd);
    return 0;
}
