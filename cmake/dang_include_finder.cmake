# Forwards to include() and makes sure the config also contains this include().
macro(dang_include_finder)
  include(${ARGN})
  string(REPLACE ";" " " FINDER "include(${ARGN})")
  if(NOT ${FINDER} IN_LIST DANG_FINDERS)
    set(DANG_FINDERS ${DANG_FINDERS} ${FINDER} CACHE INTERNAL "finder")
  endif()
endmacro()
