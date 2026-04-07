/************************************************************************
 * ai_app — normalize Software Bus HK payloads into feature vectors
 *
 * Layout: first payload_len bytes (capped by feature_dim) scaled to [0,1].
 * Mission-specific struct layouts should replace this mapping in flight builds.
 ************************************************************************/
#ifndef AI_APP_TELEMETRY_H
#define AI_APP_TELEMETRY_H

#include <stddef.h>
#include <stdint.h>

#include "ai_app_mission_cfg.h"

void AI_APP_NormalizePayloadToFeatures(const uint8_t *payload, size_t payload_len, double *features, size_t feature_dim);

#endif /* AI_APP_TELEMETRY_H */
