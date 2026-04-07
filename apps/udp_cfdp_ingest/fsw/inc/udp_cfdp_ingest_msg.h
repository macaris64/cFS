/************************************************************************
 * udp_cfdp_ingest message construction helpers (testable)
 ************************************************************************/
#ifndef UDP_CFDP_INGEST_MSG_H
#define UDP_CFDP_INGEST_MSG_H

#include <stddef.h>
#include <stdint.h>

/*
 * Unit-test mode: avoid pulling full cFE/OSAL include graph.
 * Flight builds include <cfe.h>.
 */
#ifdef UDP_CFDP_INGEST_UNIT_TEST
typedef int32_t CFE_Status_t;
typedef uint16_t CFE_SB_MsgId_t;
typedef size_t CFE_MSG_Size_t;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef struct
{
    uint8_t bytes[12];
} CFE_MSG_CommandHeader_t;
typedef struct
{
    uint8_t bytes[16];
} CFE_MSG_Message_t;
typedef struct
{
    CFE_MSG_Message_t Msg;
} CFE_SB_Buffer_t;

#define CFE_SUCCESS 0
#define CFE_STATUS_WRONG_MSG_LENGTH (-1)
#define CFE_SB_BUF_ALOC_ERR (-2)
#define CFE_ES_BAD_ARGUMENT (-3)

#define CFE_MSG_PTR(x) ((CFE_MSG_Message_t *)&(x))
#define CFE_SB_ValueToMsgId(v) ((CFE_SB_MsgId_t)(v))
#define CFE_SB_MsgIdToValue(v) ((uint16_t)(v))

CFE_SB_Buffer_t *CFE_SB_AllocateMessageBuffer(CFE_MSG_Size_t MsgSize);
void CFE_SB_ReleaseMessageBuffer(CFE_SB_Buffer_t *Buf);
CFE_Status_t CFE_MSG_Init(CFE_MSG_Message_t *MsgPtr, CFE_SB_MsgId_t MsgId, CFE_MSG_Size_t Size);
#else
#include "cfe.h"
#endif

CFE_Status_t UDP_CFDP_INGEST_BuildPduSbBuffer(const uint8 *pdu, size_t pdu_len, CFE_SB_Buffer_t **out_buf);

#endif /* UDP_CFDP_INGEST_MSG_H */

