/************************************************************************
 * bridge_reader — Software Bus subscription for Rust CCSDS bridge packets.
 ************************************************************************/

#include <stdio.h>
#include <string.h>

#include "cfe.h"

#include "bridge_reader_ccsds.h"
#include "bridge_reader_crc.h"
#include "bridge_reader_internal_cfg.h"
#include "bridge_reader_mission_ids.h"
#include "bridge_reader_perfids.h"
#include "bridge_reader_platform_cfg.h"
#include "bridge_reader_version.h"

/*
 * MsgIds to subscribe to — add entries here as the mission expands.
 * Kept as an array so subscription stays easy to manage.
 */
static const uint16_t BRIDGE_READER_SubscriptionMsgValues[] = {
    BRIDGE_SB_MSGID_RAW_VALUE,
};

static const size_t BRIDGE_READER_SubscriptionCount =
    sizeof(BRIDGE_READER_SubscriptionMsgValues) / sizeof(BRIDGE_READER_SubscriptionMsgValues[0]);

static void BRIDGE_READER_ProcessBridgeWire(const uint8_t *wire, size_t wire_len);
static void BRIDGE_READER_LogPayloadHex(const uint8_t *payload, size_t payload_len);

/*
 * Payload length from CCSDS Packet Data Length field (matches rust-bridge).
 */
static size_t BridgeReader_PayloadLenFromDataLengthField(uint16_t w2)
{
    if (w2 == 0xFFFFu)
    {
        return 0;
    }
    return (size_t)w2 + 1u;
}

/*
 * Entry symbol must be <= 20 characters (cFE ES loader truncates lookup).
 */
void BRIDGE_READER_Main(void)
{
    CFE_Status_t     status;
    uint32           RunStatus = CFE_ES_RunStatus_APP_RUN;
    CFE_SB_Buffer_t *SBBufPtr;
    CFE_SB_PipeId_t  PipeId;
    size_t           i;

    CFE_ES_PerfLogEntry(BRIDGE_READER_MAIN_TASK_PERF_ID);

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("BRIDGE_READER: EVS Register failed RC = 0x%08lX\n", (unsigned long)status);
    }

    status = CFE_SB_CreatePipe(&PipeId, BRIDGE_READER_PLATFORM_PIPE_DEPTH, "BR_RD_PIPE");
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("BRIDGE_READER: CreatePipe failed RC = 0x%08lX\n", (unsigned long)status);
        RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    if (status == CFE_SUCCESS)
    {
        for (i = 0; i < BRIDGE_READER_SubscriptionCount; i++)
        {
            status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(BRIDGE_READER_SubscriptionMsgValues[i]), PipeId);
            if (status != CFE_SUCCESS)
            {
                CFE_ES_WriteToSysLog("BRIDGE_READER: Subscribe 0x%X failed RC = 0x%08lX\n",
                                     (unsigned int)BRIDGE_READER_SubscriptionMsgValues[i], (unsigned long)status);
                RunStatus = CFE_ES_RunStatus_APP_ERROR;
                break;
            }
        }
    }

    if (status == CFE_SUCCESS)
    {
        OS_printf("BRIDGE_READER: Initialized v%u.%u.%u, subscribed to bridge MsgId 0x%X\n",
                  BRIDGE_READER_MAJOR_VERSION, BRIDGE_READER_MINOR_VERSION, BRIDGE_READER_REVISION,
                  (unsigned int)BRIDGE_SB_MSGID_RAW_VALUE);
        CFE_ES_WriteToSysLog("BRIDGE_READER: Initialized, subscribed to MsgId 0x%X\n",
                             (unsigned int)BRIDGE_SB_MSGID_RAW_VALUE);
    }

    while (CFE_ES_RunLoop(&RunStatus) == true)
    {
        CFE_ES_PerfLogExit(BRIDGE_READER_MAIN_TASK_PERF_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, PipeId, BRIDGE_READER_PLATFORM_SB_RECEIVE_TIMEOUT);

        CFE_ES_PerfLogEntry(BRIDGE_READER_MAIN_TASK_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            CFE_MSG_Size_t           sz    = 0;
            size_t                   hdr_sz = sizeof(CFE_MSG_Message_t);
            const uint8_t *          wire;
            size_t                   wire_len;

            CFE_MSG_GetSize(&SBBufPtr->Msg, &sz);
            if (sz < (CFE_MSG_Size_t)hdr_sz)
            {
                CFE_ES_WriteToSysLog("BRIDGE_READER: message too small (%u)\n", (unsigned int)sz);
                continue;
            }

            wire_len = (size_t)sz - hdr_sz;
            wire     = (const uint8_t *)&SBBufPtr->Msg + hdr_sz;

            BRIDGE_READER_ProcessBridgeWire(wire, wire_len);
        }
    }

    CFE_ES_PerfLogExit(BRIDGE_READER_MAIN_TASK_PERF_ID);
}

static void BRIDGE_READER_LogPayloadHex(const uint8_t *payload, size_t payload_len)
{
    char   line[160];
    size_t pos = 0;
    size_t i;

    pos = 0;
    for (i = 0; i < payload_len && pos + 4 < sizeof(line); i++)
    {
        int n = snprintf(&line[pos], sizeof(line) - pos, "%02X%s", (unsigned int)payload[i],
                         (i + 1 < payload_len) ? " " : "");
        if (n <= 0)
        {
            break;
        }
        pos += (size_t)n;
    }
    line[sizeof(line) - 1] = '\0';

    OS_printf("Bridge Reader: Received valid packet. Payload: [%s]\n", line);
    CFE_ES_WriteToSysLog("Bridge Reader: Received valid packet. Payload: [%s]\n", line);
}

static void BRIDGE_READER_ProcessBridgeWire(const uint8_t *wire, size_t wire_len)
{
    BridgeReader_CcsdsPrimaryHeader_t pri;
    uint16_t                          apid;
    uint8_t                           version;
    uint8_t                           packet_type;
    bool                              sec_hdr;
    uint8_t                           seq_flags;
    uint16_t                          seq_count;
    uint16_t                          w2;
    size_t                            payload_len;
    uint16_t                          crc_calc;
    uint16_t                          crc_wire;
    const uint8_t *                   payload;

    if (wire_len < 8u)
    {
        CFE_ES_WriteToSysLog("BRIDGE_READER: wire too short for CRC validation\n");
        return;
    }

    memcpy(pri.bytes, wire, sizeof(pri.bytes));
    BridgeReader_ParsePrimaryHeader(&pri, &apid, &version, &packet_type, &sec_hdr, &seq_flags, &seq_count, &w2);

    (void)version;
    (void)packet_type;
    (void)sec_hdr;
    (void)seq_flags;
    (void)seq_count;
    (void)apid;

    payload_len = BridgeReader_PayloadLenFromDataLengthField(w2);
    if (6u + payload_len + 2u != wire_len)
    {
        CFE_ES_WriteToSysLog("BRIDGE_READER: wire length mismatch (radiation/corruption?)\n");
        return;
    }

    crc_calc = BridgeReader_Crc16CcittFalse(wire, 6u + payload_len);
    crc_wire = ((uint16_t)wire[6u + payload_len] << 8) | (uint16_t)wire[6u + payload_len + 1u];
    if (crc_calc != crc_wire)
    {
        CFE_ES_WriteToSysLog(
            "BRIDGE_READER: invalid CRC-16 (expected 0x%04X, got 0x%04X), discarding packet\n",
            (unsigned int)crc_calc, (unsigned int)crc_wire);
        return;
    }

    payload = wire + 6u;
    BRIDGE_READER_LogPayloadHex(payload, payload_len);
}
