# ai_app — MicroGPT-style inference core + cFS SB HK ingestion
#
# Full cFS bundle: add to sample_defs/targets.cmake, e.g.
#   list(APPEND MISSION_GLOBAL_APPLIST ai_app)
# (same pattern as bridge_reader in cfs/.github/workflows/unit-test-coverage.yml)
#
project(CFS_AI_APP C)

set(AI_APP_SRC_FILES
  fsw/src/ai_app_app.c
  fsw/src/ai_app_autograd.c
  fsw/src/ai_app_tensor.c
  fsw/src/ai_app_gpt.c
  fsw/src/ai_app_telemetry.c
  fsw/src/ai_app_mission_subscribe.c
  fsw/src/ai_app_tbl.c
  fsw/src/ai_app_tbl_mgr.c
)

add_cfe_app(ai_app ${AI_APP_SRC_FILES})

target_include_directories(ai_app PUBLIC fsw/inc)

# `ai_app_gpt.c` uses libm symbols (e.g. `exp`, `sqrt`) on Linux.
target_link_libraries(ai_app m)

# Host-side tests (no cFE): from repo root of app, run: make -C unit-test check
# Full cFS bundle with ENABLE_UNIT_TESTS uses per-module coverage tests (see nasa/sample_app).
if (DEFINED ENABLE_UNIT_TESTS AND ENABLE_UNIT_TESTS AND COMMAND add_cfe_coverage_test)
  add_subdirectory(unit-test)
endif ()
