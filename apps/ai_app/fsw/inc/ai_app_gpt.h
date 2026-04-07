/************************************************************************
 * ai_app — microgpt-style transformer forward (inference, double precision)
 ************************************************************************/
#ifndef AI_APP_GPT_H
#define AI_APP_GPT_H

#include <stddef.h>
#include <stdint.h>

#include "ai_app_mission_cfg.h"

typedef struct {
    const double *wte;
    const double *wpe;
    const double *lm_head;
    const double *attn_wq;
    const double *attn_wk;
    const double *attn_wv;
    const double *attn_wo;
    const double *mlp_fc1;
    const double *mlp_fc2;
} AI_APP_GptLayerWeights;

typedef struct {
    size_t            vocab_size;
    size_t            n_embd;
    size_t            block_size;
    size_t            n_head;
    size_t            n_layer;
    const double *    wte;
    const double *    wpe;
    const double *    lm_head;
    AI_APP_GptLayerWeights layers[AI_APP_GPT_MAX_LAYER];
} AI_APP_GptWeights;

typedef struct {
    int32_t kv_len[AI_APP_GPT_MAX_LAYER];
} AI_APP_GptState;

void AI_APP_GptState_Init(AI_APP_GptState *st);

/*
 * One autoregressive step at position pos_id (0 .. block_size-1).
 * Appends projected K/V for each layer into internal static buffers keyed by pos_id.
 */
int AI_APP_GptForwardStep(const AI_APP_GptWeights *weights, AI_APP_GptState *st, uint32_t token_id, uint32_t pos_id,
                          double *logits_out);

#endif /* AI_APP_GPT_H */
