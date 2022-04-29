# Adds the subdirectory if the given feature is enabled.
function(dang_add_feature_subdirectory FEATURE SUBDIRECTORY)
  if(WITH_DANG_${FEATURE})
    add_subdirectory(${SUBDIRECTORY})
  endif()
endfunction()
