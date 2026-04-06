/************************************************************************
 * Bridge reader / Rust bridge — shared numeric IDs
 *
 * BRIDGE_SB_MSGID_RAW_VALUE is the cFS Software Bus MsgId (subscription).
 * BRIDGE_WIRE_CCSDS_APID is the 11-bit APID in the Rust CCSDS primary header.
 ************************************************************************/
#ifndef BRIDGE_READER_MISSION_IDS_H
#define BRIDGE_READER_MISSION_IDS_H

/*
 * Must NOT equal CFE_ES_CMD_MID (0x1800|6 = 0x1806) or other core CMD MsgIds — ES would
 * treat bridge payloads as executive commands and emit length errors.
 * Use an unused platform command topic (here 0xF0): 0x1800 | 0xF0 = 0x18F0.
 */
#define BRIDGE_SB_MSGID_RAW_VALUE 0x18F0u
#define BRIDGE_WIRE_CCSDS_APID 0x006u

#endif /* BRIDGE_READER_MISSION_IDS_H */
