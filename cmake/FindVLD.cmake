if(MSVC)
  option(WITH_VLD "With Visual Leak Detector")

  if(WITH_VLD)
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
      message("-- Enabling Visual Leak Detector for Win32")
      find_library(VLD vld HINTS "$ENV{PROGRAMFILES\(x86\)}/Visual Leak Detector/lib/Win32"
                                 "$ENV{PROGRAMFILES}/Visual Leak Detector/lib/Win32")
    elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
      message("-- Enabling Visual Leak Detector for Win64")
      find_library(VLD vld HINTS "$ENV{PROGRAMFILES\(x86\)}/Visual Leak Detector/lib/Win64"
                                 "$ENV{PROGRAMFILES}/Visual Leak Detector/lib/Win64")
    else()
      message(FATAL_ERROR "Only Win32 and Win64 support Visual Leak Detector.")
    endif()

    find_file(VLD_H vld.h HINTS "$ENV{PROGRAMFILES\(x86\)}/Visual Leak Detector/include"
                                "$ENV{PROGRAMFILES}/Visual Leak Detector/include")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \"/FI${VLD_H}\"")

    get_filename_component(VLD_LIBRARY_DIR "${VLD}" PATH)
    link_directories("${VLD_LIBRARY_DIR}")

    add_compile_definitions(VLD_FORCE_ENABLE)
  endif()
endif()
