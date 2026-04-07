/************************************************************************
 * ai_app GPT forward test (weights from LCG — must match scripts/golden/gpt_forward_golden.py)
 ************************************************************************/

#include "ai_app_gpt.h"

#include <stdint.h>
#include <string.h>

#include "utassert_stubs.h"

static uint32_t s_lcg = 1234567u;

static double lcg_next_f(void)
{
    s_lcg = s_lcg * 1103515245u + 12345u;
    s_lcg &= 0x7FFFFFFFu;
    return ((double)s_lcg / (double)0x7FFFFFFF) * 0.16 - 0.08;
}

static void fill_mat(double *out, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++)
    {
        out[i] = lcg_next_f();
    }
}

void UtTest_Gpt_Run(void)
{
    static double wte[17 * 16];
    static double wpe[16 * 16];
    static double lm_head[17 * 16];
    static double attn_wq[16 * 16];
    static double attn_wk[16 * 16];
    static double attn_wv[16 * 16];
    static double attn_wo[16 * 16];
    static double mlp_fc1[64 * 16];
    static double mlp_fc2[16 * 64];

    AI_APP_GptWeights w;
    AI_APP_GptState   st;
    double            logits[17];
    const double      exp0[5] = {0.037957028509855467, -0.43107678771360347, 0.091857601513958165, 0.28609067272997435,
                                 0.27857731863067503};
    size_t            i;

    memset(&w, 0, sizeof(w));
    s_lcg = 1234567u;
    fill_mat(wte, sizeof(wte) / sizeof(wte[0]));
    fill_mat(wpe, sizeof(wpe) / sizeof(wpe[0]));
    fill_mat(lm_head, sizeof(lm_head) / sizeof(lm_head[0]));
    fill_mat(attn_wq, sizeof(attn_wq) / sizeof(attn_wq[0]));
    fill_mat(attn_wk, sizeof(attn_wk) / sizeof(attn_wk[0]));
    fill_mat(attn_wv, sizeof(attn_wv) / sizeof(attn_wv[0]));
    fill_mat(attn_wo, sizeof(attn_wo) / sizeof(attn_wo[0]));
    fill_mat(mlp_fc1, sizeof(mlp_fc1) / sizeof(mlp_fc1[0]));
    fill_mat(mlp_fc2, sizeof(mlp_fc2) / sizeof(mlp_fc2[0]));

    w.vocab_size = 17;
    w.n_embd     = 16;
    w.block_size = 16;
    w.n_head     = 4;
    w.n_layer    = 1;
    w.wte        = wte;
    w.wpe        = wpe;
    w.lm_head    = lm_head;
    w.layers[0].attn_wq = attn_wq;
    w.layers[0].attn_wk = attn_wk;
    w.layers[0].attn_wv = attn_wv;
    w.layers[0].attn_wo = attn_wo;
    w.layers[0].mlp_fc1 = mlp_fc1;
    w.layers[0].mlp_fc2 = mlp_fc2;

    UtAssert_True(AI_APP_GptForwardStep(&w, &st, 3u, 0u, logits) == 0, "gpt step rc");
    for (i = 0; i < 5; i++)
    {
        UtAssert_DoubleCmpAbs(logits[i], exp0[i], 1e-9, "logit");
    }
}
