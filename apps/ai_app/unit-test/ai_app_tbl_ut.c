/************************************************************************
 * ai_app table logic unit tests (host, no cFE required)
 ************************************************************************/

#include "ai_app_tbl.h"
#include "ai_app_tbl_mgr.h"

#include <math.h>
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

void UtTest_Tbl_Run(void)
{
    AI_APP_WeightsTable_t tbl;
    char                 reason[128];
    AI_APP_TblMgr_t      mgr;
    uint32_t             tok;
    uint32_t             pos;

    memset(&tbl, 0, sizeof(tbl));
    tbl.Hdr.Magic     = AI_APP_WEIGHTS_TBL_MAGIC;
    tbl.Hdr.Version   = AI_APP_WEIGHTS_TBL_VERSION;
    tbl.Hdr.VocabSize = AI_APP_GPT_VOCAB_SIZE;
    tbl.Hdr.NEmbd     = AI_APP_GPT_N_EMBD;
    tbl.Hdr.BlockSize = AI_APP_GPT_BLOCK_SIZE;
    tbl.Hdr.NHead     = AI_APP_GPT_N_HEAD;
    tbl.Hdr.NLayer    = 1u;
    (void)strncpy(tbl.Hdr.MissionVersion, "UT", sizeof(tbl.Hdr.MissionVersion) - 1u);

    /* Fill deterministic weights (same distribution as gpt UT) */
    s_lcg = 1234567u;
    fill_mat(tbl.wte, sizeof(tbl.wte) / sizeof(tbl.wte[0]));
    fill_mat(tbl.wpe, sizeof(tbl.wpe) / sizeof(tbl.wpe[0]));
    fill_mat(tbl.lm_head, sizeof(tbl.lm_head) / sizeof(tbl.lm_head[0]));
    fill_mat(tbl.attn_wq[0], sizeof(tbl.attn_wq[0]) / sizeof(tbl.attn_wq[0][0]));
    fill_mat(tbl.attn_wk[0], sizeof(tbl.attn_wk[0]) / sizeof(tbl.attn_wk[0][0]));
    fill_mat(tbl.attn_wv[0], sizeof(tbl.attn_wv[0]) / sizeof(tbl.attn_wv[0][0]));
    fill_mat(tbl.attn_wo[0], sizeof(tbl.attn_wo[0]) / sizeof(tbl.attn_wo[0][0]));
    fill_mat(tbl.mlp_fc1[0], sizeof(tbl.mlp_fc1[0]) / sizeof(tbl.mlp_fc1[0][0]));
    fill_mat(tbl.mlp_fc2[0], sizeof(tbl.mlp_fc2[0]) / sizeof(tbl.mlp_fc2[0][0]));

    /* Set CRC to a value that will fail to verify specific-reason reporting */
    tbl.Hdr.Crc32 = 0xDEADBEEFu;
    UtAssert_True(AI_APP_ValidateWeightsRaw(&tbl, reason, sizeof(reason)) != 0, "CRC mismatch rejected");
    UtAssert_True(strstr(reason, "CRC mismatch") != NULL, "CRC reason text");

    /* Now corrupt with NaN and ensure reason mentions the location */
    tbl.wte[3]    = 0.0;
    tbl.Hdr.Crc32 = AI_APP_WeightsTable_CalcCrc32(&tbl);
    tbl.wte[3]    = NAN;
    tbl.Hdr.Crc32 = AI_APP_WeightsTable_CalcCrc32(&tbl);
    UtAssert_True(AI_APP_ValidateWeightsRaw(&tbl, reason, sizeof(reason)) != 0, "NaN rejected");
    UtAssert_True(strstr(reason, "wte[3]") != NULL, "NaN index in reason");

    /* Deterministic token/pos mapping sanity (does not require cFE) */
    AI_APP_TblMgr_Init(&mgr, NULL, NULL);
    {
        double features[AI_APP_TLM_FEATURE_DIM];
        size_t i;
        for (i = 0; i < AI_APP_TLM_FEATURE_DIM; i++)
        {
            features[i] = (double)i / (double)(AI_APP_TLM_FEATURE_DIM);
        }
        AI_APP_TblMgr_DeriveTokenPos(&mgr, features, AI_APP_TLM_FEATURE_DIM, AI_APP_GPT_VOCAB_SIZE, AI_APP_GPT_BLOCK_SIZE, &tok,
                                     &pos);
        UtAssert_True(tok < AI_APP_GPT_VOCAB_SIZE, "token bound");
        UtAssert_True(pos < AI_APP_GPT_BLOCK_SIZE, "pos bound");
    }
}

