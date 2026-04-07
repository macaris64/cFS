/**
 * @file
 * ai_app table-based model weight definition (cFE Table Services)
 */
#ifndef AI_APP_TBL_H
#define AI_APP_TBL_H

#include <stddef.h>
#include <stdint.h>

#include "ai_app_mission_cfg.h"

/*
 * Table integrity header. Magic/version allow quick rejection of wrong images.
 * CRC32 is computed over the table bytes excluding the Crc32 field itself.
 */
#define AI_APP_WEIGHTS_TBL_MAGIC (0x41495F4150505F57ULL) /* "AI_APP_W" */
#define AI_APP_WEIGHTS_TBL_VERSION (1u)

/* Mission version string carried inside table for ground tracking */
#define AI_APP_WEIGHTS_TBL_MISSION_VER_LEN (64u)

/* Conservative safety bounds for weights (mission may tighten/override) */
#define AI_APP_WT_MIN (-1000.0)
#define AI_APP_WT_MAX (1000.0)

typedef struct
{
    uint64_t Magic;
    uint32_t Version;
    uint32_t Crc32;
    uint32_t VocabSize;
    uint32_t NEmbd;
    uint32_t BlockSize;
    uint32_t NHead;
    uint32_t NLayer;
    char     MissionVersion[AI_APP_WEIGHTS_TBL_MISSION_VER_LEN];
} AI_APP_WeightsTblHdr_t;

typedef struct
{
    AI_APP_WeightsTblHdr_t Hdr;

    /* Embeddings and LM head */
    double wte[AI_APP_GPT_VOCAB_SIZE * AI_APP_GPT_N_EMBD];
    double wpe[AI_APP_GPT_BLOCK_SIZE * AI_APP_GPT_N_EMBD];
    double lm_head[AI_APP_GPT_VOCAB_SIZE * AI_APP_GPT_N_EMBD];

    /* Per-layer weights. Only the first Hdr.NLayer entries are used. */
    double attn_wq[AI_APP_GPT_MAX_LAYER][AI_APP_GPT_N_EMBD * AI_APP_GPT_N_EMBD];
    double attn_wk[AI_APP_GPT_MAX_LAYER][AI_APP_GPT_N_EMBD * AI_APP_GPT_N_EMBD];
    double attn_wv[AI_APP_GPT_MAX_LAYER][AI_APP_GPT_N_EMBD * AI_APP_GPT_N_EMBD];
    double attn_wo[AI_APP_GPT_MAX_LAYER][AI_APP_GPT_N_EMBD * AI_APP_GPT_N_EMBD];
    double mlp_fc1[AI_APP_GPT_MAX_LAYER][AI_APP_GPT_MLP_HIDDEN * AI_APP_GPT_N_EMBD];
    double mlp_fc2[AI_APP_GPT_MAX_LAYER][AI_APP_GPT_N_EMBD * AI_APP_GPT_MLP_HIDDEN];
} AI_APP_WeightsTable_t;

/*
 * Pure validation routine used by flight callback and host UT.
 *
 * - Returns 0 on success, non-zero on failure.
 * - On failure, Reason (if provided) will contain a short description suitable
 *   for event text (e.g. "CRC mismatch" / "NaN at wte[12]").
 */
int AI_APP_ValidateWeightsRaw(const AI_APP_WeightsTable_t *Tbl, char *Reason, size_t ReasonMax);

/* Compute CRC32 over bytes (IEEE 802.3 polynomial, reflected). */
uint32_t AI_APP_Crc32(const void *Data, size_t Len);

/* Compute CRC32 for the weight table image (skipping the Crc32 field). */
uint32_t AI_APP_WeightsTable_CalcCrc32(const AI_APP_WeightsTable_t *Tbl);

/* Compile-time alignment sanity: table size should be 8-byte multiple. */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Static_assert((sizeof(AI_APP_WeightsTable_t) % 8u) == 0u, "AI_APP_WeightsTable_t must be 64-bit padded");
#endif

#endif /* AI_APP_TBL_H */

