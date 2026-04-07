###########################################################
#
# AI_APP platform build setup
#
###########################################################

set(AI_APP_PLATFORM_CONFIG_FILE_LIST
  ai_app_internal_cfg_values.h
  ai_app_platform_cfg.h
)

generate_configfile_set(${AI_APP_PLATFORM_CONFIG_FILE_LIST})
