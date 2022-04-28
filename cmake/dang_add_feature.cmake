# Adds a new feature and adds the subdirectory if it is enabled.
# Also checks for additionally passed dependent features.
function(dang_add_feature BASE_FEATURE SUBDIRECTORY DESCRIPTION)
  option(WITH_DANG_${BASE_FEATURE} ${DESCRIPTION} ON)
  if(WITH_DANG_${BASE_FEATURE})
    foreach(FEATURE IN LISTS ARGN)
      if(NOT WITH_DANG_${FEATURE})
        message(SEND_ERROR "-- The feature WITH_DANG_${BASE_FEATURE} also requires WITH_DANG_${FEATURE}.")
      endif()
    endforeach()
    add_subdirectory(${SUBDIRECTORY})
  endif()
endfunction()
