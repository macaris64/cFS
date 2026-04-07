/************************************************************************
 * ai_app — cFS application entry (HK subscriptions, feature extraction hook)
 ************************************************************************/

#include <string.h>

#include "cfe.h"

#include "osapi.h"

#include "ai_app_events.h"
#include "ai_app_internal_cfg.h"
#include "ai_app_gpt.h"
#include "ai_app_mission_subscribe.h"
#include "ai_app_perfids.h"
#include "ai_app_platform_cfg.h"
#include "ai_app_tbl.h"
#include "ai_app_tbl_mgr.h"
#include "ai_app_telemetry.h"
#include "ai_app_version.h"

static void AI_APP_ProcessSbBuffer(CFE_SB_Buffer_t *SBBufPtr);

static CFE_TBL_Handle_t g_WeightsTblHandle;
static AI_APP_TblMgr_t  g_TblMgr;
static AI_APP_GptState  g_GptState;

static void AI_APP_BuildWeightsView(const AI_APP_WeightsTable_t *t, AI_APP_GptWeights *w)
{
    size_t li;
    if (t == NULL || w == NULL)
    {
        return;
    }
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

static void AI_APP_SendTblValidateCritical(const char *Text)
{
    if (Text == NULL)
    {
        CFE_EVS_SendEvent(AI_APP_EVT_TBL_VALIDATE_CRIT, CFE_EVS_EventType_CRITICAL,
                          "AI_APP weights table validation failed");
    }
    else
    {
        CFE_EVS_SendEvent(AI_APP_EVT_TBL_VALIDATE_CRIT, CFE_EVS_EventType_CRITICAL,
                          "AI_APP weights table validation failed: %s", Text);
    }
}

static int32 AI_APP_ValidateWeights(void *TblPtr)
{
    char reason[128];
    const AI_APP_WeightsTable_t *tbl = (const AI_APP_WeightsTable_t *)TblPtr;

    if (AI_APP_ValidateWeightsRaw(tbl, reason, sizeof(reason)) != 0)
    {
        /* Per requirement: emit CRITICAL with specific reason */
        AI_APP_SendTblValidateCritical(reason);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }
    CFE_EVS_SendEvent(AI_APP_EVT_TBL_VALIDATE_INF, CFE_EVS_EventType_INFORMATION,
                      "AI_APP weights table validation success");
    return CFE_SUCCESS;
}

void AI_APP_Main(void)
{
    CFE_Status_t          status;
    uint32                RunStatus = CFE_ES_RunStatus_APP_RUN;
    CFE_SB_Buffer_t *     SBBufPtr;
    CFE_SB_PipeId_t       PipeId;
    const uint16_t *      sub_vals;
    size_t                sub_count;
    size_t                i;

    CFE_ES_PerfLogEntry(AI_APP_MAIN_TASK_PERF_ID);

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("AI_APP: EVS Register failed RC = 0x%08lX\n", (unsigned long)status);
    }

    status = CFE_SB_CreatePipe(&PipeId, AI_APP_PLATFORM_PIPE_DEPTH, "AI_APP_PIPE");
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("AI_APP: CreatePipe failed RC = 0x%08lX\n", (unsigned long)status);
        RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    if (status == CFE_SUCCESS)
    {
        AI_APP_TblMgr_Init(&g_TblMgr, NULL, NULL);

        /* Base name only: cFE forms the registry key as AppName.BaseName → "AI_APP.WEIGHTS". */
        status = CFE_TBL_Register(&g_WeightsTblHandle, "WEIGHTS", sizeof(AI_APP_WeightsTable_t), CFE_TBL_OPT_DEFAULT,
                                  AI_APP_ValidateWeights);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(AI_APP_EVT_TBL_REGISTER_ERR, CFE_EVS_EventType_CRITICAL,
                              "AI_APP: CFE_TBL_Register(WEIGHTS) failed RC=0x%08lX", (unsigned long)status);
            RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    if (status == CFE_SUCCESS)
    {
        sub_count = AI_APP_Mission_GetSubscriptionSbValues(&sub_vals);
        for (i = 0; i < sub_count; i++)
        {
            status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(sub_vals[i]), PipeId);
            if (status != CFE_SUCCESS)
            {
                CFE_ES_WriteToSysLog("AI_APP: Subscribe 0x%X failed RC = 0x%08lX\n",
                                     (unsigned int)sub_vals[i], (unsigned long)status);
                RunStatus = CFE_ES_RunStatus_APP_ERROR;
                break;
            }
        }
    }

    if (status == CFE_SUCCESS)
    {
        OS_printf("AI_APP: Initialized v%u.%u.%u, subscribed to %zu HK topic(s)\n", AI_APP_MAJOR_VERSION,
                  AI_APP_MINOR_VERSION, AI_APP_REVISION, AI_APP_Mission_GetSubscriptionSbValues(NULL));
        CFE_ES_WriteToSysLog("AI_APP: Initialized\n");
        CFE_EVS_SendEvent(AI_APP_EVT_INIT_INF, CFE_EVS_EventType_INFORMATION, "AI_APP initialized v%u.%u.%u",
                          AI_APP_MAJOR_VERSION, AI_APP_MINOR_VERSION, AI_APP_REVISION);
    }

    while (CFE_ES_RunLoop(&RunStatus) == true)
    {
        CFE_ES_PerfLogExit(AI_APP_MAIN_TASK_PERF_ID);

        /* Safe sync point for table management (background update/double-buffering) */
        status = CFE_TBL_Manage(g_WeightsTblHandle);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(AI_APP_EVT_TBL_MANAGE_ERR, CFE_EVS_EventType_ERROR,
                              "AI_APP: CFE_TBL_Manage failed RC=0x%08lX",
                              (unsigned long)status);
        }

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, PipeId, AI_APP_PLATFORM_SB_RECEIVE_TIMEOUT);

        CFE_ES_PerfLogEntry(AI_APP_MAIN_TASK_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            AI_APP_ProcessSbBuffer(SBBufPtr);
        }
        else if (status != CFE_SB_NO_MESSAGE && status != CFE_SB_TIME_OUT)
        {
            CFE_ES_WriteToSysLog("AI_APP: ReceiveBuffer unexpected RC = 0x%08lX\n", (unsigned long)status);
        }
    }

    CFE_ES_PerfLogExit(AI_APP_MAIN_TASK_PERF_ID);
    CFE_ES_ExitApp(RunStatus);
}

static void AI_APP_ProcessSbBuffer(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_Size_t sz         = 0;
    size_t         hdr_sz     = sizeof(CFE_MSG_Message_t);
    const uint8_t *payload;
    size_t         payload_len;
    double         features[AI_APP_TLM_FEATURE_DIM];
    const AI_APP_WeightsTable_t *tbl = NULL;
    AI_APP_GptWeights            w;
    uint32_t                     token_id = 0u;
    uint32_t                     pos_id   = 0u;
    double                       logits[AI_APP_GPT_VOCAB_SIZE];
    int                          gpt_rc;
    OS_time_t                    t0;
    OS_time_t                    t1;
    OS_time_t                    dt;
    int64                        dt_us;

    if (SBBufPtr == NULL)
    {
        return;
    }

    CFE_MSG_GetSize(&SBBufPtr->Msg, &sz);
    if (sz <= (CFE_MSG_Size_t)hdr_sz)
    {
        return;
    }

    payload_len = (size_t)sz - hdr_sz;
    payload     = (const uint8_t *)&SBBufPtr->Msg + hdr_sz;

    AI_APP_NormalizePayloadToFeatures(payload, payload_len, features, AI_APP_TLM_FEATURE_DIM);

    {
        CFE_Status_t tbl_rc = CFE_TBL_GetAddress((void **)&tbl, g_WeightsTblHandle);
        if (tbl_rc != CFE_SUCCESS && tbl_rc != CFE_TBL_INFO_UPDATED)
        {
            CFE_EVS_SendEvent(AI_APP_EVT_TBL_GET_ERR, CFE_EVS_EventType_CRITICAL,
                              "AI_APP: CFE_TBL_GetAddress failed RC=0x%08lX", (unsigned long)tbl_rc);
            return;
        }
    }

    /* Safety belt: re-validate active buffer (detect memory corruption) */
    {
        char reason[128];
        if (AI_APP_ValidateWeightsRaw(tbl, reason, sizeof(reason)) != 0)
        {
            AI_APP_SendTblValidateCritical(reason);
            (void)CFE_TBL_ReleaseAddress(g_WeightsTblHandle);
            return;
        }
    }

    if (tbl == NULL)
    {
        CFE_EVS_SendEvent(AI_APP_EVT_TBL_GET_ERR, CFE_EVS_EventType_CRITICAL,
                          "AI_APP: weights table unavailable/corrupt");
        return;
    }
    AI_APP_BuildWeightsView(tbl, &w);

    AI_APP_TblMgr_DeriveTokenPos(&g_TblMgr, features, AI_APP_TLM_FEATURE_DIM, (uint32_t)w.vocab_size,
                                 (uint32_t)w.block_size, &token_id, &pos_id);

    (void)OS_GetLocalTime(&t0);
    gpt_rc = AI_APP_GptForwardStep(&w, &g_GptState, token_id, pos_id, logits);
    (void)OS_GetLocalTime(&t1);

    dt    = OS_TimeSubtract(t1, t0);
    dt_us = OS_TimeGetTotalMicroseconds(dt);
    if (dt_us < 0)
    {
        dt_us = 0;
    }

    if (CFE_TBL_ReleaseAddress(g_WeightsTblHandle) != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(AI_APP_EVT_TBL_RELEASE_ERR, CFE_EVS_EventType_ERROR,
                          "AI_APP: CFE_TBL_ReleaseAddress failed");
    }

    if (gpt_rc != 0)
    {
        CFE_EVS_SendEvent(AI_APP_EVT_INFER_FAIL_CRIT, CFE_EVS_EventType_CRITICAL,
                          "AI_APP: inference failed rc=%d token=%lu pos=%lu", gpt_rc, (unsigned long)token_id,
                          (unsigned long)pos_id);
        return;
    }

    CFE_EVS_SendEvent(AI_APP_EVT_INFER_TIME_INF, CFE_EVS_EventType_INFORMATION,
                      "AI_APP: inference dt_us=%lld token=%lu pos=%lu logit0=%.6g", (long long)dt_us,
                      (unsigned long)token_id, (unsigned long)pos_id, logits[0]);
}
