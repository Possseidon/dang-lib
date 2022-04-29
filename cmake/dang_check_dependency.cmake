# Checks if all dependent features are enabled.
macro(dang_check_dependency)
  foreach(DEPENDENCY ${ARGN})
    if(NOT WITH_DANG_${DEPENDENCY})
      message(SEND_ERROR "${PROJECT_NAME} requires feature WITH_DANG_${DEPENDENCY}")
    endif()
  endforeach()
endmacro()
