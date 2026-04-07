/************************************************************************
 * Host-side unit test for UDP_CFDP_INGEST_BuildPduSbBuffer()
 *
 * This test stubs the minimal SB/MSG functions used by the helper and checks:
 * - MsgId used is CF channel 0 input MID (0x18C8)
 * - Payload bytes are copied verbatim at the correct offset
 ************************************************************************/

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "udp_cfdp_ingest_msg.h"
#include "udp_cfdp_ingest_platform_cfg.h"

/* --- Minimal stubs for cFE SB/MSG allocation/init used by the helper --- */

static uint8 g_buf_storage[4096];
static size_t g_last_alloc = 0;
static CFE_SB_Buffer_t *g_last_buf = NULL;
static CFE_SB_MsgId_t g_last_msgid;

CFE_SB_Buffer_t *CFE_SB_AllocateMessageBuffer(CFE_MSG_Size_t MsgSize)
{
    if ((size_t)MsgSize > sizeof(g_buf_storage))
    {
        return NULL;
    }
    memset(g_buf_storage, 0, sizeof(g_buf_storage));
    g_last_alloc = (size_t)MsgSize;
    g_last_buf = (CFE_SB_Buffer_t *)g_buf_storage;
    return g_last_buf;
}

void CFE_SB_ReleaseMessageBuffer(CFE_SB_Buffer_t *Buf)
{
    (void)Buf;
    g_last_buf = NULL;
}

CFE_Status_t CFE_MSG_Init(CFE_MSG_Message_t *MsgPtr, CFE_SB_MsgId_t MsgId, CFE_MSG_Size_t Size)
{
    (void)MsgPtr;
    (void)Size;
    g_last_msgid = MsgId;
    return CFE_SUCCESS;
}

int main(void)
{
    CFE_Status_t rc;
    CFE_SB_Buffer_t *buf = NULL;
    uint8 pdu[8] = {0x20, 0x00, 0x00, 0x13, 0x00, 0x19, 0xAA, 0xBB};

    rc = UDP_CFDP_INGEST_BuildPduSbBuffer(pdu, sizeof(pdu), &buf);
    assert(rc == CFE_SUCCESS);
    assert(buf != NULL);

    /* Ensure we used the expected CF channel 0 input MID value */
    assert(CFE_SB_MsgIdToValue(g_last_msgid) == UDP_CFDP_INGEST_CF_CH0_INPUT_MIDVAL);

    /* Verify payload starts immediately after CommandHeader */
    {
        uint8 *base = (uint8 *)&buf->Msg;
        size_t off = sizeof(CFE_MSG_CommandHeader_t);
        assert(off + sizeof(pdu) <= g_last_alloc);
        assert(memcmp(base + off, pdu, sizeof(pdu)) == 0);
    }

    return 0;
}

