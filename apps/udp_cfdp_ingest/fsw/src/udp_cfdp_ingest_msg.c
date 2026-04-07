/************************************************************************
 * udp_cfdp_ingest message construction helpers (testable)
 ************************************************************************/

#include "udp_cfdp_ingest_msg.h"
#include "udp_cfdp_ingest_platform_cfg.h"

#include <string.h>

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 Payload[UDP_CFDP_INGEST_MAX_DATAGRAM];
} UDP_CFDP_INGEST_PduCmd_t;

CFE_Status_t UDP_CFDP_INGEST_BuildPduSbBuffer(const uint8 *pdu, size_t pdu_len, CFE_SB_Buffer_t **out_buf)
{
    CFE_SB_Buffer_t *Buf;
    UDP_CFDP_INGEST_PduCmd_t *msg;
    size_t total;

    if (out_buf == NULL)
    {
        return CFE_ES_BAD_ARGUMENT;
    }
    *out_buf = NULL;

    if (pdu == NULL || pdu_len == 0 || pdu_len > UDP_CFDP_INGEST_MAX_DATAGRAM)
    {
        return CFE_STATUS_WRONG_MSG_LENGTH;
    }

    total = offsetof(UDP_CFDP_INGEST_PduCmd_t, Payload) + pdu_len;
    Buf   = CFE_SB_AllocateMessageBuffer(total);
    if (Buf == NULL)
    {
        return CFE_SB_BUF_ALOC_ERR;
    }

    /*
     * IMPORTANT: initialize the message at the actual message base (`Buf->Msg`).
     * CF distinguishes CMD vs TLM encapsulation using `CFE_MSG_GetType(&Buf->Msg)`, so the
     * header must be initialized consistently with the expected on-wire layout.
     */
    if (CFE_MSG_Init(&Buf->Msg, CFE_SB_ValueToMsgId(UDP_CFDP_INGEST_CF_CH0_INPUT_MIDVAL), total) != CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer(Buf);
        return CFE_STATUS_WRONG_MSG_LENGTH;
    }

    msg = (UDP_CFDP_INGEST_PduCmd_t *)&Buf->Msg;
    memcpy(msg->Payload, pdu, pdu_len);

    *out_buf = Buf;
    return CFE_SUCCESS;
}

