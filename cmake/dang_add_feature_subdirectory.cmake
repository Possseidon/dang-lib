# Adds the subdirectory if the given feature is enabled.
function(dang_add_feature_subdirectory feature subdirectory)
  if(WITH_DANG_${feature})
    add_subdirectory(${subdirectory})
  endif()
endfunction()
