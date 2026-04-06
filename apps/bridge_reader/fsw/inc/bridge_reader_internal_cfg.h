/************************************************************************
 * BRIDGE_READER private configuration
 ************************************************************************/
#ifndef BRIDGE_READER_INTERNAL_CFG_H
#define BRIDGE_READER_INTERNAL_CFG_H

#include "bridge_reader_mission_cfg.h"
#include "bridge_reader_internal_cfg_values.h"

#define BRIDGE_READER_PLATFORM_PIPE_DEPTH BRIDGE_READER_PLATFORM_CFGVAL(PIPE_DEPTH)
#define DEFAULT_BRIDGE_READER_PLATFORM_PIPE_DEPTH 32

#define BRIDGE_READER_PLATFORM_SB_RECEIVE_TIMEOUT BRIDGE_READER_PLATFORM_CFGVAL(SB_RECEIVE_TIMEOUT)
#define DEFAULT_BRIDGE_READER_PLATFORM_SB_RECEIVE_TIMEOUT 500

#endif
