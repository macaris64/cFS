/************************************************************************
 * ai_app mission MsgId / subscription values
 *
 * Full cFS missions provide CFE_ES_HK_TLM_MID / SENSORS_HK_TLM_MID via
 * cfe_msgids.h and mission tables. Lab builds use numeric SB values here.
 ************************************************************************/
#ifndef AI_APP_MISSION_IDS_H
#define AI_APP_MISSION_IDS_H

#include <stdint.h>

/*
 * Default ES Executive Services HK telemetry (typical open-source cFS lab value).
 * Replace in mission configuration if your MsgId map differs.
 */
#define AI_APP_SB_VALUE_CFE_ES_HK_TLM 0x0803u

/*
 * Placeholder sensors housekeeping telemetry — define SENSORS_HK_TLM_MID in
 * mission headers to override routing in integrated builds.
 */
#define AI_APP_SB_VALUE_SENSORS_HK_TLM 0x0820u

#endif /* AI_APP_MISSION_IDS_H */
