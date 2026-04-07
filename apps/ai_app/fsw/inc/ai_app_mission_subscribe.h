/************************************************************************
 * ai_app — Software Bus subscription table (testable without stubs)
 ************************************************************************/
#ifndef AI_APP_MISSION_SUBSCRIBE_H
#define AI_APP_MISSION_SUBSCRIBE_H

#include <stddef.h>
#include <stdint.h>

size_t AI_APP_Mission_GetSubscriptionSbValues(const uint16_t **out_vals);

#endif /* AI_APP_MISSION_SUBSCRIBE_H */
