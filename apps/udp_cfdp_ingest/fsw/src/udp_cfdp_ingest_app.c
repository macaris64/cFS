/************************************************************************
 * udp_cfdp_ingest: UDP datagram (raw CFDP PDU) → SB message for CF channel RX
 *
 * Lab-only bridge that bypasses CI_LAB. Each UDP datagram payload is treated as
 * a complete CFDP PDU byte sequence (starting at CFDP common header flags byte).
 ************************************************************************/

#include "cfe.h"
#include "osapi.h"

#include <string.h>

#include "udp_cfdp_ingest_platform_cfg.h"
#include "udp_cfdp_ingest_msg.h"

static osal_id_t UDP_CFDP_INGEST_OpenListenSocket(uint16 port)
{
    osal_id_t sock;
    int32 status;
    OS_SockAddr_t addr;

    sock = OS_OBJECT_ID_UNDEFINED;

    memset(&addr, 0, sizeof(addr));
    if (OS_SocketAddrInit(&addr, OS_SocketDomain_INET) != OS_SUCCESS)
    {
        return OS_OBJECT_ID_UNDEFINED;
    }
    if (OS_SocketAddrSetPort(&addr, port) != OS_SUCCESS)
    {
        return OS_OBJECT_ID_UNDEFINED;
    }

    status = OS_SocketOpen(&sock, OS_SocketDomain_INET, OS_SocketType_DATAGRAM);
    if (status != OS_SUCCESS)
    {
        return OS_OBJECT_ID_UNDEFINED;
    }

    status = OS_SocketBind(sock, &addr);
    if (status != OS_SUCCESS)
    {
        (void)OS_close(sock);
        return OS_OBJECT_ID_UNDEFINED;
    }

    return sock;
}

static CFE_Status_t UDP_CFDP_INGEST_PublishPdu(const uint8 *pdu, size_t pdu_len)
{
    CFE_SB_Buffer_t *Buf = NULL;
    CFE_Status_t rc;

    rc = UDP_CFDP_INGEST_BuildPduSbBuffer(pdu, pdu_len, &Buf);
    if (rc != CFE_SUCCESS)
    {
        return rc;
    }
    return CFE_SB_TransmitBuffer(Buf, true);
}

void UDP_CFDP_INGEST_AppMain(void)
{
    CFE_Status_t status;
    osal_id_t sock;
    uint8 rxbuf[UDP_CFDP_INGEST_MAX_DATAGRAM];
    OS_SockAddr_t from;
    int32 n;
    uint32 recv_count = 0;

    CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);

    sock = UDP_CFDP_INGEST_OpenListenSocket(UDP_CFDP_INGEST_LISTEN_PORT);
    if (!OS_ObjectIdDefined(sock))
    {
        CFE_EVS_SendEvent(0x9001, CFE_EVS_EventType_CRITICAL,
                          "UDP_CFDP_INGEST: socket open/bind failed port=%u", (unsigned)UDP_CFDP_INGEST_LISTEN_PORT);
        CFE_ES_ExitApp(CFE_ES_RunStatus_APP_ERROR);
    }

    CFE_EVS_SendEvent(0x9000, CFE_EVS_EventType_INFORMATION,
                      "UDP_CFDP_INGEST: listening UDP port=%u -> CF MID 0x%04X",
                      (unsigned)UDP_CFDP_INGEST_LISTEN_PORT, (unsigned)UDP_CFDP_INGEST_CF_CH0_INPUT_MIDVAL);

    while (CFE_ES_RunLoop(&(uint32){CFE_ES_RunStatus_APP_RUN}) == true)
    {
        memset(&from, 0, sizeof(from));
        n = OS_SocketRecvFrom(sock, rxbuf, sizeof(rxbuf), &from, OS_PEND);
        if (n <= 0)
        {
            continue;
        }

        recv_count++;
        if (recv_count == 1 || (recv_count % 100) == 0)
        {
            CFE_EVS_SendEvent(0x9003, CFE_EVS_EventType_INFORMATION,
                              "UDP_CFDP_INGEST: received datagram #%lu len=%ld",
                              (unsigned long)recv_count, (long)n);
        }

        status = UDP_CFDP_INGEST_PublishPdu(rxbuf, (size_t)n);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(0x9002, CFE_EVS_EventType_ERROR,
                              "UDP_CFDP_INGEST: publish failed rc=0x%08lX len=%ld",
                              (unsigned long)status, (long)n);
        }
    }

    (void)OS_close(sock);
    CFE_ES_ExitApp(CFE_ES_RunStatus_APP_RUN);
}

/*
 * Some cFE startup scripts / loaders expect an `*_App` symbol rather than `*_AppMain`.
 * Provide a tiny alias to avoid brittle startup-script coupling.
 */
void UDP_CFDP_INGEST_App(void)
{
    UDP_CFDP_INGEST_AppMain();
}

