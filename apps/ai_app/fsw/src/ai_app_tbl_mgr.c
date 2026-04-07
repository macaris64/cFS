/************************************************************************
 * ai_app table manager (cFE-free logic, ops injected by flight app / UT)
 ************************************************************************/

#include "ai_app_tbl_mgr.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static void safe_snprintf(char *dst, size_t dst_max, const char *src)
{
    size_t i;
    if (dst == NULL || dst_max == 0u)
    {
        return;
    }
    if (src == NULL)
    {
        dst[0] = '\0';
        return;
    }
    for (i = 0; i + 1u < dst_max && src[i] != '\0'; i++)
    {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

static uint32_t crc32_update(uint32_t crc, uint8_t byte)
{
    uint32_t       bit;
    crc ^= (uint32_t)byte;
    for (bit = 0u; bit < 8u; bit++)
    {
        if ((crc & 1u) != 0u)
        {
            crc = (crc >> 1) ^ 0xEDB88320u;
        }
        else
        {
            crc >>= 1;
        }
    }
    return crc;
}

uint32_t AI_APP_Crc32(const void *Data, size_t Len)
{
    /* IEEE 802.3 CRC32, reflected, init=0xFFFFFFFF, xorout=0xFFFFFFFF */
    uint32_t       crc = 0xFFFFFFFFu;
    const uint8_t *p   = (const uint8_t *)Data;
    size_t         i;

    if (p == NULL)
    {
        return 0u;
    }

    for (i = 0; i < Len; i++)
    {
        crc = crc32_update(crc, p[i]);
    }

    return crc ^ 0xFFFFFFFFu;
}

static int is_bad_double(double x)
{
    /* isfinite is C99; keep as a single helper for consistency */
    return (isfinite(x) == 0);
}

static int check_range(double x)
{
    return (x < AI_APP_WT_MIN || x > AI_APP_WT_MAX);
}

int AI_APP_ValidateWeightsRaw(const AI_APP_WeightsTable_t *Tbl, char *Reason, size_t ReasonMax)
{
    size_t i;
    size_t li;
    uint32_t crc_calc;
    const uint8_t *bytes;
    size_t crc_off;
    size_t table_sz;
    size_t crc_end;
    uint32_t crc_work;

    if (Reason != NULL && ReasonMax > 0u)
    {
        Reason[0] = '\0';
    }

    if (Tbl == NULL)
    {
        safe_snprintf(Reason, ReasonMax, "NULL table pointer");
        return -1;
    }

    if (Tbl->Hdr.Magic != AI_APP_WEIGHTS_TBL_MAGIC)
    {
        safe_snprintf(Reason, ReasonMax, "Bad magic");
        return -1;
    }
    if (Tbl->Hdr.Version != AI_APP_WEIGHTS_TBL_VERSION)
    {
        safe_snprintf(Reason, ReasonMax, "Bad version");
        return -1;
    }

    if (Tbl->Hdr.VocabSize != (uint32_t)AI_APP_GPT_VOCAB_SIZE || Tbl->Hdr.NEmbd != (uint32_t)AI_APP_GPT_N_EMBD ||
        Tbl->Hdr.BlockSize != (uint32_t)AI_APP_GPT_BLOCK_SIZE || Tbl->Hdr.NHead != (uint32_t)AI_APP_GPT_N_HEAD)
    {
        safe_snprintf(Reason, ReasonMax, "Dim mismatch");
        return -1;
    }
    if (Tbl->Hdr.NLayer == 0u || Tbl->Hdr.NLayer > (uint32_t)AI_APP_GPT_MAX_LAYER)
    {
        safe_snprintf(Reason, ReasonMax, "Bad NLayer");
        return -1;
    }

    /* CRC32 over whole table excluding the Crc32 field bytes */
    bytes    = (const uint8_t *)Tbl;
    table_sz = sizeof(*Tbl);
    crc_off  = offsetof(AI_APP_WeightsTable_t, Hdr) + offsetof(AI_APP_WeightsTblHdr_t, Crc32);
    crc_end  = crc_off + sizeof(uint32_t);

    /*
     * Special-case: allow the compiled-in default image to omit CRC (Crc32==0)
     * for bring-up. Operational table updates must provide a non-zero CRC32.
     */
    if (Tbl->Hdr.Crc32 == 0u)
    {
        if (strncmp(Tbl->Hdr.MissionVersion, "DEFAULT_", 8) != 0)
        {
            safe_snprintf(Reason, ReasonMax, "CRC missing (Crc32==0)");
            return -1;
        }
        /* proceed to NaN/Inf/range scans without CRC enforcement */
        goto weight_scans;
    }

    if (bytes == NULL)
    {
        safe_snprintf(Reason, ReasonMax, "Bad bytes");
        return -1;
    }
    if (crc_end > table_sz)
    {
        safe_snprintf(Reason, ReasonMax, "CRC field offset");
        return -1;
    }

    crc_work = 0xFFFFFFFFu;
    for (i = 0; i < table_sz; i++)
    {
        if (i >= crc_off && i < crc_end)
        {
            continue;
        }
        crc_work = crc32_update(crc_work, bytes[i]);
    }
    crc_calc = crc_work ^ 0xFFFFFFFFu;

    if (crc_calc != Tbl->Hdr.Crc32)
    {
        (void)snprintf(Reason, ReasonMax, "CRC mismatch (calc=0x%08lX tbl=0x%08lX)", (unsigned long)crc_calc,
                       (unsigned long)Tbl->Hdr.Crc32);
        return -1;
    }

weight_scans:
    /* Weight scans: NaN/Inf + bounds, with index-specific reason */
    for (i = 0; i < (sizeof(Tbl->wte) / sizeof(Tbl->wte[0])); i++)
    {
        if (is_bad_double(Tbl->wte[i]))
        {
            (void)snprintf(Reason, ReasonMax, "NaN/Inf detected at wte[%lu]", (unsigned long)i);
            return -1;
        }
        if (check_range(Tbl->wte[i]))
        {
            (void)snprintf(Reason, ReasonMax, "Out-of-range at wte[%lu]=%.17g", (unsigned long)i, Tbl->wte[i]);
            return -1;
        }
    }
    for (i = 0; i < (sizeof(Tbl->wpe) / sizeof(Tbl->wpe[0])); i++)
    {
        if (is_bad_double(Tbl->wpe[i]))
        {
            (void)snprintf(Reason, ReasonMax, "NaN/Inf detected at wpe[%lu]", (unsigned long)i);
            return -1;
        }
        if (check_range(Tbl->wpe[i]))
        {
            (void)snprintf(Reason, ReasonMax, "Out-of-range at wpe[%lu]=%.17g", (unsigned long)i, Tbl->wpe[i]);
            return -1;
        }
    }
    for (i = 0; i < (sizeof(Tbl->lm_head) / sizeof(Tbl->lm_head[0])); i++)
    {
        if (is_bad_double(Tbl->lm_head[i]))
        {
            (void)snprintf(Reason, ReasonMax, "NaN/Inf detected at lm_head[%lu]", (unsigned long)i);
            return -1;
        }
        if (check_range(Tbl->lm_head[i]))
        {
            (void)snprintf(Reason, ReasonMax, "Out-of-range at lm_head[%lu]=%.17g", (unsigned long)i, Tbl->lm_head[i]);
            return -1;
        }
    }

    for (li = 0; li < (size_t)Tbl->Hdr.NLayer; li++)
    {
        for (i = 0; i < (sizeof(Tbl->attn_wq[0]) / sizeof(Tbl->attn_wq[0][0])); i++)
        {
            if (is_bad_double(Tbl->attn_wq[li][i]))
            {
                (void)snprintf(Reason, ReasonMax, "NaN/Inf at attn_wq[L%lu][%lu]", (unsigned long)li, (unsigned long)i);
                return -1;
            }
            if (check_range(Tbl->attn_wq[li][i]))
            {
                (void)snprintf(Reason, ReasonMax, "Out-of-range at attn_wq[L%lu][%lu]=%.17g", (unsigned long)li,
                               (unsigned long)i, Tbl->attn_wq[li][i]);
                return -1;
            }
        }
        for (i = 0; i < (sizeof(Tbl->attn_wk[0]) / sizeof(Tbl->attn_wk[0][0])); i++)
        {
            if (is_bad_double(Tbl->attn_wk[li][i]))
            {
                (void)snprintf(Reason, ReasonMax, "NaN/Inf at attn_wk[L%lu][%lu]", (unsigned long)li, (unsigned long)i);
                return -1;
            }
            if (check_range(Tbl->attn_wk[li][i]))
            {
                (void)snprintf(Reason, ReasonMax, "Out-of-range at attn_wk[L%lu][%lu]=%.17g", (unsigned long)li,
                               (unsigned long)i, Tbl->attn_wk[li][i]);
                return -1;
            }
        }
        for (i = 0; i < (sizeof(Tbl->attn_wv[0]) / sizeof(Tbl->attn_wv[0][0])); i++)
        {
            if (is_bad_double(Tbl->attn_wv[li][i]))
            {
                (void)snprintf(Reason, ReasonMax, "NaN/Inf at attn_wv[L%lu][%lu]", (unsigned long)li, (unsigned long)i);
                return -1;
            }
            if (check_range(Tbl->attn_wv[li][i]))
            {
                (void)snprintf(Reason, ReasonMax, "Out-of-range at attn_wv[L%lu][%lu]=%.17g", (unsigned long)li,
                               (unsigned long)i, Tbl->attn_wv[li][i]);
                return -1;
            }
        }
        for (i = 0; i < (sizeof(Tbl->attn_wo[0]) / sizeof(Tbl->attn_wo[0][0])); i++)
        {
            if (is_bad_double(Tbl->attn_wo[li][i]))
            {
                (void)snprintf(Reason, ReasonMax, "NaN/Inf at attn_wo[L%lu][%lu]", (unsigned long)li, (unsigned long)i);
                return -1;
            }
            if (check_range(Tbl->attn_wo[li][i]))
            {
                (void)snprintf(Reason, ReasonMax, "Out-of-range at attn_wo[L%lu][%lu]=%.17g", (unsigned long)li,
                               (unsigned long)i, Tbl->attn_wo[li][i]);
                return -1;
            }
        }
        for (i = 0; i < (sizeof(Tbl->mlp_fc1[0]) / sizeof(Tbl->mlp_fc1[0][0])); i++)
        {
            if (is_bad_double(Tbl->mlp_fc1[li][i]))
            {
                (void)snprintf(Reason, ReasonMax, "NaN/Inf at mlp_fc1[L%lu][%lu]", (unsigned long)li, (unsigned long)i);
                return -1;
            }
            if (check_range(Tbl->mlp_fc1[li][i]))
            {
                (void)snprintf(Reason, ReasonMax, "Out-of-range at mlp_fc1[L%lu][%lu]=%.17g", (unsigned long)li,
                               (unsigned long)i, Tbl->mlp_fc1[li][i]);
                return -1;
            }
        }
        for (i = 0; i < (sizeof(Tbl->mlp_fc2[0]) / sizeof(Tbl->mlp_fc2[0][0])); i++)
        {
            if (is_bad_double(Tbl->mlp_fc2[li][i]))
            {
                (void)snprintf(Reason, ReasonMax, "NaN/Inf at mlp_fc2[L%lu][%lu]", (unsigned long)li, (unsigned long)i);
                return -1;
            }
            if (check_range(Tbl->mlp_fc2[li][i]))
            {
                (void)snprintf(Reason, ReasonMax, "Out-of-range at mlp_fc2[L%lu][%lu]=%.17g", (unsigned long)li,
                               (unsigned long)i, Tbl->mlp_fc2[li][i]);
                return -1;
            }
        }
    }

    return 0;
}

uint32_t AI_APP_WeightsTable_CalcCrc32(const AI_APP_WeightsTable_t *Tbl)
{
    const uint8_t *bytes;
    size_t         table_sz;
    size_t         crc_off;
    size_t         crc_end;
    size_t         i;
    uint32_t       crc_work;

    if (Tbl == NULL)
    {
        return 0u;
    }

    bytes    = (const uint8_t *)Tbl;
    table_sz = sizeof(*Tbl);
    crc_off  = offsetof(AI_APP_WeightsTable_t, Hdr) + offsetof(AI_APP_WeightsTblHdr_t, Crc32);
    crc_end  = crc_off + sizeof(uint32_t);
    if (crc_end > table_sz)
    {
        return 0u;
    }

    crc_work = 0xFFFFFFFFu;
    for (i = 0; i < table_sz; i++)
    {
        if (i >= crc_off && i < crc_end)
        {
            continue;
        }
        crc_work = crc32_update(crc_work, bytes[i]);
    }
    return crc_work ^ 0xFFFFFFFFu;
}

void AI_APP_TblMgr_Init(AI_APP_TblMgr_t *Mgr, void *TblHandleStorage, const AI_APP_TblOps_t *Ops)
{
    if (Mgr == NULL)
    {
        return;
    }

    memset(Mgr, 0, sizeof(*Mgr));
    Mgr->TblHandle   = TblHandleStorage;
    Mgr->PosCounter  = 0u;
    Mgr->LastTokenId = 0u;

    if (Ops != NULL)
    {
        Mgr->Ops = *Ops;
    }
}

int32_t AI_APP_TblMgr_Register(AI_APP_TblMgr_t *Mgr, const char *Name, int32_t (*Validate)(void *TblPtr))
{
    if (Mgr == NULL || Mgr->Ops.Register == NULL)
    {
        return -1;
    }
    return Mgr->Ops.Register(Mgr->TblHandle, Name, sizeof(AI_APP_WeightsTable_t), 0u, Validate);
}

int32_t AI_APP_TblMgr_Manage(AI_APP_TblMgr_t *Mgr)
{
    if (Mgr == NULL || Mgr->Ops.Manage == NULL)
    {
        return -1;
    }
    return Mgr->Ops.Manage(Mgr->TblHandle);
}

static void build_view(const AI_APP_WeightsTable_t *t, AI_APP_GptWeights *w)
{
    size_t li;
    memset(w, 0, sizeof(*w));
    w->vocab_size = (size_t)t->Hdr.VocabSize;
    w->n_embd     = (size_t)t->Hdr.NEmbd;
    w->block_size = (size_t)t->Hdr.BlockSize;
    w->n_head     = (size_t)t->Hdr.NHead;
    w->n_layer    = (size_t)t->Hdr.NLayer;
    w->wte        = t->wte;
    w->wpe        = t->wpe;
    w->lm_head    = t->lm_head;
    for (li = 0; li < w->n_layer && li < (size_t)AI_APP_GPT_MAX_LAYER; li++)
    {
        w->layers[li].attn_wq = t->attn_wq[li];
        w->layers[li].attn_wk = t->attn_wk[li];
        w->layers[li].attn_wv = t->attn_wv[li];
        w->layers[li].attn_wo = t->attn_wo[li];
        w->layers[li].mlp_fc1 = t->mlp_fc1[li];
        w->layers[li].mlp_fc2 = t->mlp_fc2[li];
    }
}

int32_t AI_APP_TblMgr_AcquireWeights(AI_APP_TblMgr_t *Mgr, const AI_APP_WeightsTable_t **TblOut, AI_APP_GptWeights *ViewOut)
{
    void *tbl_ptr = NULL;
    int32_t rc;
    char reason[96];

    if (Mgr == NULL || TblOut == NULL || ViewOut == NULL || Mgr->Ops.GetAddress == NULL)
    {
        return -1;
    }

    rc = Mgr->Ops.GetAddress(&tbl_ptr, Mgr->TblHandle);
    if (rc != 0 || tbl_ptr == NULL)
    {
        return -1;
    }

    *TblOut = (const AI_APP_WeightsTable_t *)tbl_ptr;

    if (AI_APP_ValidateWeightsRaw(*TblOut, reason, sizeof(reason)) != 0)
    {
        if (Mgr->Ops.SendCriticalEvent != NULL)
        {
            Mgr->Ops.SendCriticalEvent(reason);
        }
        (void)AI_APP_TblMgr_Release(Mgr);
        return -1;
    }

    build_view(*TblOut, ViewOut);
    return 0;
}

int32_t AI_APP_TblMgr_Release(AI_APP_TblMgr_t *Mgr)
{
    if (Mgr == NULL || Mgr->Ops.ReleaseAddress == NULL)
    {
        return -1;
    }
    return Mgr->Ops.ReleaseAddress(Mgr->TblHandle);
}

void AI_APP_TblMgr_DeriveTokenPos(AI_APP_TblMgr_t *Mgr, const double *Features, size_t FeatureDim, uint32_t VocabSize,
                                 uint32_t BlockSize, uint32_t *TokenId, uint32_t *PosId)
{
    double acc;
    size_t i;
    uint32_t tok;
    uint32_t pos;

    if (TokenId == NULL || PosId == NULL)
    {
        return;
    }

    tok = 0u;
    pos = 0u;
    if (Mgr != NULL)
    {
        pos = (BlockSize == 0u) ? 0u : (Mgr->PosCounter % BlockSize);
        Mgr->PosCounter++;
    }

    acc = 0.0;
    if (Features != NULL && FeatureDim > 0u)
    {
        /* Stable folding: quantize each feature to 0..255 and mix */
        for (i = 0; i < FeatureDim; i++)
        {
            double x = Features[i];
            int32_t q;
            if (x < 0.0)
            {
                x = 0.0;
            }
            if (x > 1.0)
            {
                x = 1.0;
            }
            q = (int32_t)(x * 255.0 + 0.5);
            acc += (double)((q * 1315423911) ^ (int32_t)i);
        }
    }

    if (VocabSize != 0u)
    {
        /* Deterministic reduction */
        tok = (uint32_t)(((uint64_t)(acc < 0.0 ? -acc : acc)) % (uint64_t)VocabSize);
    }

    if (Mgr != NULL)
    {
        Mgr->LastTokenId = tok;
    }

    *TokenId = tok;
    *PosId   = pos;
}

