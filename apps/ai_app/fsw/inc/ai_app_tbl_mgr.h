/**
 * @file
 * ai_app table lifecycle manager (wraps cFE table ops for testability)
 */
#ifndef AI_APP_TBL_MGR_H
#define AI_APP_TBL_MGR_H

#include <stddef.h>
#include <stdint.h>

#include "ai_app_gpt.h"
#include "ai_app_tbl.h"

typedef struct
{
    /* cFE-like status codes are passed through as int32 for portability */
    int32_t (*Register)(void *TblHandle, const char *Name, size_t Size, uint32_t Options,
                        int32_t (*Validate)(void *TblPtr));
    int32_t (*Load)(void *TblHandle, int32_t SrcType, const char *SrcData);
    int32_t (*Manage)(void *TblHandle);
    int32_t (*GetAddress)(void **TblPtr, void *TblHandle);
    int32_t (*ReleaseAddress)(void *TblHandle);

    /* Optional hooks (may be NULL) */
    void (*SendCriticalEvent)(const char *Text);
} AI_APP_TblOps_t;

typedef struct
{
    void *           TblHandle;
    AI_APP_TblOps_t  Ops;
    uint32_t         PosCounter;
    uint32_t         LastTokenId;
} AI_APP_TblMgr_t;

void AI_APP_TblMgr_Init(AI_APP_TblMgr_t *Mgr, void *TblHandleStorage, const AI_APP_TblOps_t *Ops);

int32_t AI_APP_TblMgr_Register(AI_APP_TblMgr_t *Mgr, const char *Name, int32_t (*Validate)(void *TblPtr));

int32_t AI_APP_TblMgr_Manage(AI_APP_TblMgr_t *Mgr);

/*
 * Acquire weights table and build AI_APP_GptWeights view.
 * Returns 0 on success; caller must call AI_APP_TblMgr_Release() after use.
 */
int32_t AI_APP_TblMgr_AcquireWeights(AI_APP_TblMgr_t *Mgr, const AI_APP_WeightsTable_t **TblOut, AI_APP_GptWeights *ViewOut);

int32_t AI_APP_TblMgr_Release(AI_APP_TblMgr_t *Mgr);

/*
 * Deterministic mapping from normalized features [0,1] to token/pos.
 * - token_id derived from a stable quantization of features.
 * - pos_id increments monotonically and wraps at block_size.
 */
void AI_APP_TblMgr_DeriveTokenPos(AI_APP_TblMgr_t *Mgr, const double *Features, size_t FeatureDim, uint32_t VocabSize,
                                 uint32_t BlockSize, uint32_t *TokenId, uint32_t *PosId);

#endif /* AI_APP_TBL_MGR_H */

