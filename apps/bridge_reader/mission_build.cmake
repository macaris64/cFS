###########################################################
#
# BRIDGE_READER mission build setup
#
###########################################################

set(BRIDGE_READER_MISSION_CONFIG_FILE_LIST
  bridge_reader_perfids.h
  bridge_reader_mission_cfg.h
  bridge_reader_interface_cfg.h
)

generate_configfile_set(${BRIDGE_READER_MISSION_CONFIG_FILE_LIST})
