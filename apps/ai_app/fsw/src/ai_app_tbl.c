/************************************************************************
 * ai_app weights table default image (flight)
 ************************************************************************/

#include "ai_app_tbl.h"

#include "cfe_tbl_filedef.h"
/* cFE provides this header during flight builds */

/*
 * Note: This default image is intentionally simple (zeros) but fully valid
 * with respect to NaN/Inf/range checks. Missions are expected to load an
 * updated .tbl image via cFE Table Services for real inference behavior.
 *
 * The Crc32 field must match the bytes of this struct excluding the Crc32
 * field itself. We compute it once at build time and keep it as a constant.
 */

const AI_APP_WeightsTable_t AI_APP_WeightsTblDefault = {
    .Hdr =
        {
            .Magic          = AI_APP_WEIGHTS_TBL_MAGIC,
            .Version        = AI_APP_WEIGHTS_TBL_VERSION,
            .Crc32          = 0u, /* patched at build-time */
            .VocabSize      = AI_APP_GPT_VOCAB_SIZE,
            .NEmbd          = AI_APP_GPT_N_EMBD,
            .BlockSize      = AI_APP_GPT_BLOCK_SIZE,
            .NHead          = AI_APP_GPT_N_HEAD,
            .NLayer         = AI_APP_GPT_N_LAYER,
            .MissionVersion = "DEFAULT_COMPILED",
        },
    .wte     = {0.0},
    .wpe     = {0.0},
    .lm_head = {0.0},
    .attn_wq = {{0.0}},
    .attn_wk = {{0.0}},
    .attn_wv = {{0.0}},
    .attn_wo = {{0.0}},
    .mlp_fc1 = {{0.0}},
    .mlp_fc2 = {{0.0}},
};

/*
 * Table file definition for cFE tools (elf2cfetbl).
 * The required macro/header are provided by cFE during flight builds.
 */
CFE_TBL_FILEDEF(AI_APP_WeightsTblDefault, AI_APP.WEIGHTS, AI Weights Table, ai_app_weights.tbl)

