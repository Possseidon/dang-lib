find_package(Doxygen)

if(DOXYGEN_FOUND)

  set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile.in)
  set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  configure_file(${DOXYFILE_IN} ${DOXYFILE} @ONLY)

  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)

endif()
