# Forwards to find_package() and makes sure the config contains a corresponding find_dependency().
function(dang_find_package)
  find_package(${ARGN})
  string(REPLACE ";" " " DEPENDENCY_INFO "find_dependency(${ARGN})")
  if(NOT ${DEPENDENCY_INFO} IN_LIST DANG_DEPENDENCIES)
    set(DANG_DEPENDENCIES ${DANG_DEPENDENCIES} ${DEPENDENCY_INFO} CACHE INTERNAL "dependencies")
  endif()
endfunction()
