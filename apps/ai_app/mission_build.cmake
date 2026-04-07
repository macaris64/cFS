###########################################################
#
# AI_APP mission build setup
#
###########################################################

set(AI_APP_MISSION_CONFIG_FILE_LIST
  ai_app_perfids.h
  ai_app_mission_cfg.h
  ai_app_interface_cfg.h
)

generate_configfile_set(${AI_APP_MISSION_CONFIG_FILE_LIST})
