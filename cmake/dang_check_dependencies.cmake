# Checks if all dependent features are enabled.
macro(DANG_CHECK_DEPENDENCIES)
  foreach(dependency ${ARGN})
    if(NOT WITH_DANG_${dependency})
      message(SEND_ERROR "${PROJECT_NAME} requires feature WITH_DANG_${dependency}")
    endif()
  endforeach()
endmacro()
