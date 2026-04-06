project(CFS_BRIDGE_READER C)

add_cfe_app(bridge_reader
  fsw/src/bridge_reader_app.c
  fsw/src/bridge_reader_crc.c
)

target_include_directories(bridge_reader PUBLIC fsw/inc)
