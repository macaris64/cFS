/************************************************************************
 * udp_cfdp_ingest platform configuration (lab default)
 ************************************************************************/
#ifndef UDP_CFDP_INGEST_PLATFORM_CFG_H
#define UDP_CFDP_INGEST_PLATFORM_CFG_H

/* Dedicated UDP port for incoming raw CFDP PDUs (bypasses CI_LAB). */
#define UDP_CFDP_INGEST_LISTEN_PORT 5235

/* Receive buffer size for one datagram (must accommodate max CFDP PDU). */
#define UDP_CFDP_INGEST_MAX_DATAGRAM 2048

/* CF channel 0 input MID from CF default config table (`cf_def_config.c`). */
#define UDP_CFDP_INGEST_CF_CH0_INPUT_MIDVAL 0x18C8u

#endif /* UDP_CFDP_INGEST_PLATFORM_CFG_H */

