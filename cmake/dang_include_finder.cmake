# Forwards to include() and makes sure the config also contains this include().
macro(DANG_INCLUDE_FINDER)
  include(${ARGN})
  string(REPLACE ";" " " FINDER "include(${ARGN})")
  if(NOT ${FINDER} IN_LIST _DANG_FINDERS)
    set(_DANG_FINDERS
        ${_DANG_FINDERS} ${FINDER}
        CACHE INTERNAL "finder")
  endif()
endmacro()
