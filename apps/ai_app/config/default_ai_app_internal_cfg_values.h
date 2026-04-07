/************************************************************************
 * ai_app internal configuration values (defaults)
 *
 * This header provides the `AI_APP_PLATFORM_CFGVAL(x)` indirection used by
 * `fsw/inc/ai_app_internal_cfg.h` in this repo's simplified app layout.
 ************************************************************************/
#ifndef DEFAULT_AI_APP_INTERNAL_CFG_VALUES_H
#define DEFAULT_AI_APP_INTERNAL_CFG_VALUES_H

/*
 * Resolve config keys to concrete numeric values.
 *
 * Keeping these in one place makes it easy to override in a mission tree
 * without modifying the flight source.
 */
#define AI_APP_PLATFORM_CFGVAL(name) AI_APP_PLATFORM_CFGVAL_##name

#define AI_APP_PLATFORM_CFGVAL_PIPE_DEPTH 32
#define AI_APP_PLATFORM_CFGVAL_SB_RECEIVE_TIMEOUT 500

#endif /* DEFAULT_AI_APP_INTERNAL_CFG_VALUES_H */

