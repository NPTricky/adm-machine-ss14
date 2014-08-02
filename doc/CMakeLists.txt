# -----------------------------------------------------------------------------
# Documentation
# -----------------------------------------------------------------------------

find_package(Doxygen REQUIRED)

if(DOXYGEN_FOUND)
  # Options
  option(DOXYGEN_BUILD_XML "Build Documentation (XML)" OFF)
  option(DOXYGEN_BUILD_MAN "Build Documentation (Man)" OFF)
  option(DOXYGEN_BUILD_RTF "Build Documentation (RTF)" OFF)
  option(DOXYGEN_BUILD_PDF "Build Documentation (PDF)" OFF)
  option(DOXYGEN_BUILD_HTML "Build Documentation (HTML)" ON)
  option(DOXYGEN_GRAPH_VISUALIZATION "Use Dot Tool (Graph Visualization)" OFF)
  option(DOXYGEN_BUILD_PDFLATEX "Use PDFLATEX" ON)
  mark_as_advanced(
    DOXYGEN_GRAPH_VISUALIZATION
    DOXYGEN_BUILD_PDFLATEX
    DOXYGEN_BUILD_MAN
    DOXYGEN_BUILD_RTF
    DOXYGEN_BUILD_XML
  )
  # Reset Configuration Defaults
  set(DOXYGEN_BUILD_PDF_CONFIG "NO")
  set(DOXYGEN_BUILD_PDFLATEX_CONFIG "NO")
  set(DOXYGEN_GRAPH_VISUALIZATION_CONFIG "NO")
  # XML Documentation
  set(DOXYGEN_BUILD_XML_CONFIG ${DOXYGEN_BUILD_XML})
  # Man Documentation
  set(DOXYGEN_BUILD_MAN_CONFIG ${DOXYGEN_BUILD_MAN})
  # RTF Documentation
  set(DOXYGEN_BUILD_RTF_CONFIG ${DOXYGEN_BUILD_RTF})
  # HTML Documentation
  set(DOXYGEN_BUILD_HTML_CONFIG ${DOXYGEN_BUILD_HTML})
  # PDF Documentation
  if(DOXYGEN_BUILD_PDF)
    find_package(LATEX)
    if(LATEX_COMPILER AND MAKEINDEX_COMPILER)
      if(DOXYGEN_BUILD_PDFLATEX AND PDFLATEX_COMPILER)
        set(DOXYGEN_BUILD_PDF_CONFIG "YES")
        set(DOXYGEN_BUILD_PDFLATEX_CONFIG "YES")
      else() # NOT DOXYGEN_BUILD_PDFLATEX AND NOT PDFLATEX_COMPILER
        if(GHOSTSCRIPT_BINARY_PATH)
          set(DOXYGEN_BUILD_PDF_CONFIG "YES")
        else()
          message(STATUS "Requirement(s) BUILD_PDF: GHOSTSCRIPT or PDFLATEX!")
        endif()
      endif()
      if(DOXYGEN_GRAPH_VISUALIZATION)
        if(DOXYGEN_DOT_EXECUTABLE)
          set(DOXYGEN_GRAPH_VISUALIZATION_CONFIG "YES")
        else() # NOT DOXYGEN_DOT_EXECUTABLE
          message(STATUS "Requirement(s) DOXYGEN_GRAPH_VISUALIZATION: DOXYGEN_DOT_EXECUTABLE!")
        endif()
      endif()
    else() # NOT LATEX_COMPILER AND NOT MAKEINDEX_COMPILER
      message(STATUS "Requirement(s) DOXYGEN_BUILD_PDF: LATEX_COMPILER and MAKEINDEX_COMPILER!")
    endif()
  endif()
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY IMMEDIATE
  )
  # Convenience Build Target
  if(DOXYGEN_BUILD_RTF OR DOXYGEN_BUILD_HTML_CONFIG OR DOXYGEN_BUILD_PDF_CONFIG)
    add_custom_target(Documentation
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      COMMENT "Generate Doxygen Documentation"
      VERBATIM
    )
  endif()
endif()
