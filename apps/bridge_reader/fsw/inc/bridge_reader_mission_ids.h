/************************************************************************
 * Bridge reader / Rust bridge — shared numeric IDs
 *
 * Each on-wire CCSDS APID maps to one cFS Software Bus MsgId (CI_LAB routing).
 * Keep rust-bridge dictionary entries and these macros aligned.
 ************************************************************************/
#ifndef BRIDGE_READER_MISSION_IDS_H
#define BRIDGE_READER_MISSION_IDS_H

/*
 * Must NOT equal CFE_ES_CMD_MID (0x1800|6 = 0x1806) or other core CMD MsgIds — ES would
 * treat bridge payloads as executive commands and emit length errors.
 * Platform command topics: 0x1800 | 0xF0 = 0x18F0, 0x1800 | 0xF1 = 0x18F1, ...
 */

/* CMD_HEARTBEAT — legacy aliases */
#define BRIDGE_WIRE_CCSDS_APID_HEARTBEAT 0x006u
#define BRIDGE_SB_MSGID_HEARTBEAT          0x18F0u

/* Second dictionary command (pilot) */
#define BRIDGE_WIRE_CCSDS_APID_PING 0x007u
#define BRIDGE_SB_MSGID_PING        0x18F1u

/* CMD_TO_LAB_ENABLE_OUTPUT — CI_LAB builds TO_LAB_EnableOutputCmd (SB MsgId TO_LAB_CMD_MID), not bridge_reader */
#define BRIDGE_WIRE_CCSDS_APID_TO_LAB_ENABLE_OUTPUT 0x008u

/* CMD_TO_LAB_DISABLE_OUTPUT — CI_LAB builds TO_LAB_DisableOutputCmd (SB MsgId TO_LAB_CMD_MID), not bridge_reader */
#define BRIDGE_WIRE_CCSDS_APID_TO_LAB_DISABLE_OUTPUT 0x009u

/* CMD_CFE_TBL_LOAD_FILE — CI_LAB builds CFE_TBL_LoadCmd (SB MsgId CFE_TBL_CMD_MID), not bridge_reader */
#define BRIDGE_WIRE_CCSDS_APID_CFE_TBL_LOAD_FILE 0x00Au

/* CMD_CFE_TBL_ACTIVATE — CI_LAB builds CFE_TBL_ActivateCmd (SB MsgId CFE_TBL_CMD_MID), not bridge_reader */
#define BRIDGE_WIRE_CCSDS_APID_CFE_TBL_ACTIVATE 0x00Bu

/* Backward-compatible names (heartbeat / first bridge topic) */
#define BRIDGE_SB_MSGID_RAW_VALUE BRIDGE_SB_MSGID_HEARTBEAT
#define BRIDGE_WIRE_CCSDS_APID    BRIDGE_WIRE_CCSDS_APID_HEARTBEAT

#endif /* BRIDGE_READER_MISSION_IDS_H */
