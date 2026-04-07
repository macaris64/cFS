project(CFS_UDP_CFDP_INGEST C)

add_cfe_app(udp_cfdp_ingest
  fsw/src/udp_cfdp_ingest_app.c
  fsw/src/udp_cfdp_ingest_msg.c
)

target_include_directories(udp_cfdp_ingest PUBLIC fsw/inc)

