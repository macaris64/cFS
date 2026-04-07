##################################################################
#
# ai_app — optional cFE coverage unit tests when building full bundle.
# Default workflow in this repo: `make -C unit-test check` (host, no stubs).
#
##################################################################

if (NOT COMMAND add_cfe_coverage_test)
  return()
endif ()

# Example pattern (uncomment and extend stubs when integrating with nasa/cFE):
# add_cfe_coverage_stubs(ai_app stubs/ai_app_stubs.c)
# add_cfe_coverage_test(ai_app ai_app_autograd "${CMAKE_CURRENT_SOURCE_DIR}/coveragetest/coveragetest_ai_app_autograd.c"
#                       "../fsw/src/ai_app_autograd.c")
