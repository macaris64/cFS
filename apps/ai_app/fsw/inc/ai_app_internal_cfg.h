/************************************************************************
 * ai_app private configuration
 ************************************************************************/
#ifndef AI_APP_INTERNAL_CFG_H
#define AI_APP_INTERNAL_CFG_H

#include "ai_app_mission_cfg.h"
#include "ai_app_internal_cfg_values.h"

#define AI_APP_PLATFORM_PIPE_DEPTH AI_APP_PLATFORM_CFGVAL(PIPE_DEPTH)
#define DEFAULT_AI_APP_PLATFORM_PIPE_DEPTH 32

#define AI_APP_PLATFORM_SB_RECEIVE_TIMEOUT AI_APP_PLATFORM_CFGVAL(SB_RECEIVE_TIMEOUT)
#define DEFAULT_AI_APP_PLATFORM_SB_RECEIVE_TIMEOUT 500

#endif /* AI_APP_INTERNAL_CFG_H */
