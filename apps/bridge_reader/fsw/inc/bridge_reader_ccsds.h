/************************************************************************
 * CCSDS primary header bit layout — mirrors rust-bridge/src/lib.rs
 * CcsdsPacket::primary_header_bytes / from_bytes (keep in sync).
 ************************************************************************/
#ifndef BRIDGE_READER_CCSDS_H
#define BRIDGE_READER_CCSDS_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Raw 6-byte CCSDS space packet primary header (big-endian 16-bit words).
 * Same field packing as Rust CcsdsPacket::primary_header_bytes.
 */
typedef struct
{
    uint8_t bytes[6];
} BridgeReader_CcsdsPrimaryHeader_t;

static inline uint16_t BridgeReader_ReadBe16(const uint8_t *p)
{
    return ((uint16_t)p[0] << 8) | (uint16_t)p[1];
}

static inline void BridgeReader_ParsePrimaryHeader(const BridgeReader_CcsdsPrimaryHeader_t *h, uint16_t *apid_out,
                                                   uint8_t *version_out, uint8_t *packet_type_out, bool *sec_hdr_out,
                                                   uint8_t *seq_flags_out, uint16_t *seq_count_out, uint16_t *data_length_field_out)
{
    uint16_t w0 = BridgeReader_ReadBe16(&h->bytes[0]);
    uint16_t w1 = BridgeReader_ReadBe16(&h->bytes[2]);
    uint16_t w2 = BridgeReader_ReadBe16(&h->bytes[4]);

    *version_out         = (uint8_t)((w0 >> 13) & 0x7u);
    *packet_type_out     = (uint8_t)((w0 >> 12) & 0x1u);
    *sec_hdr_out         = ((w0 >> 11) & 0x1u) != 0;
    *apid_out            = w0 & 0x07FFu;
    *seq_flags_out       = (uint8_t)((w1 >> 14) & 0x3u);
    *seq_count_out       = w1 & 0x3FFFu;
    *data_length_field_out = w2;
}

#endif /* BRIDGE_READER_CCSDS_H */
