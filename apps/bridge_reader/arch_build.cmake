###########################################################
#
# BRIDGE_READER platform build setup
#
###########################################################

set(BRIDGE_READER_PLATFORM_CONFIG_FILE_LIST
  bridge_reader_internal_cfg_values.h
  bridge_reader_platform_cfg.h
)

generate_configfile_set(${BRIDGE_READER_PLATFORM_CONFIG_FILE_LIST})
