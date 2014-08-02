# -----------------------------------------------------------------------------
# get_processor_count
# -----------------------------------------------------------------------------
#
# INPUT
# - (name) OUTPUT_PROCESSOR_COUNT
#
# OUTPUT
# - OUTPUT_PROCESSOR_COUNT
#     number of available processor cores
#
# -----------------------------------------------------------------------------
function(get_processor_count OUTPUT_PROCESSOR_COUNT)
	set(PROCESSOR_COUNT 1) # default
	if(UNIX)
		set(CPUINFO_FILE "/proc/cpuinfo")
		if(EXISTS "${CPUINFO_FILE}")
			file(STRINGS "${CPUINFO_FILE}" PROCESSOR_STRINGS REGEX "^processor.: [0-9]+$")
			list(LENGTH PROCESSOR_STRINGS PROCESSOR_COUNT)
		endif()
	elseif(APPLE)
		find_program(SystemProfiler "system_profiler")
		if(SystemProfiler)
			execute_process(COMMAND ${SystemProfiler} OUTPUT_VARIABLE SYSTEM_INFORMATION)
			string(REGEX REPLACE
				"^.*Total Number Of Cores: ([0-9]+).*$"
				"\\1"
				PROCESSOR_COUNT
				"${SYSTEM_INFORMATION}"
			)
		endif()
	elseif(WIN32)
		set(PROCESSOR_COUNT "$ENV{NUMBER_OF_PROCESSORS}")
	endif()
  message(STATUS "Exactly ${PROCESSOR_COUNT} thread(s) are available for the build process")
	set(${OUTPUT_PROCESSOR_COUNT} "${PROCESSOR_COUNT}" PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# check_compiler
# -----------------------------------------------------------------------------
#
# OUTPUT
# - FATAL_ERROR on unsupported compiler
#
# -----------------------------------------------------------------------------
function(check_compiler)
  set(ERROR_MESSAGE "In order to use C++11 features, this library cannot be built using a version of")
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    # i.e 17 for MSVC < VC++ 11 alias Visual Studio 12
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "17")
      message(FATAL_ERROR "${ERROR_MESSAGE} Visual Studio less than 11")
    endif()
  elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.2")
      message(FATAL_ERROR "${ERROR_MESSAGE} Clang less than 3.2")
    endif()
  elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7")
      message(FATAL_ERROR "${ERROR_MESSAGE} GCC less than 4.7")
    endif()
  endif()
  message(STATUS "Compiler was positively checked for C++11 support")
endfunction()

# -----------------------------------------------------------------------------
# glob_source_group
# -----------------------------------------------------------------------------
#
# Not the recommended way of source tree creation but fast an clean, even after
# major code restructuring.
#
# INPUT
# - FOLDER
#     the folder to glob recursively
# - (name) OUTPUT_FILE_LIST
# - (name) OUTPUT_INCLUDE_DIR_LIST
#
# OUTPUT
# - OUTPUT_FILE_LIST
#     the list of files extracted recursively from the folder
# - OUTPUT_INCLUDE_DIR_LIST
#     the list of directories containing *.h files
#
# -----------------------------------------------------------------------------
function(glob_source_group FOLDER OUTPUT_FILE_LIST OUTPUT_INCLUDE_DIR_LIST)
  # base path wont be evaluated by source_group()
  set(PATH_BASE "${FOLDER}")
  file(GLOB_RECURSE ALL_FILES ${FOLDER}/*)

  # initialise variables
  set(FILE_LIST)
  set(INCLUDE_DIR_LIST)
  
  list(APPEND INCLUDE_DIR_LIST ${PATH_BASE})

  foreach(FILE ${ALL_FILES})
    get_filename_component(PARENT_DIR "${FILE}" PATH)
    
    # strip path base from the directory of the current file
    string(REPLACE "${PATH_BASE}" "" PARENT_DIR_STRIPPED ${PARENT_DIR})
    # change /'s to \\'s
    string(REPLACE "/" "\\\\" GROUP "${PARENT_DIR_STRIPPED}")
    
    # matches: *.c / *.cc / *.cp / *.cpp / *.cxx / *.m / *.mm
    if("${FILE}" MATCHES "^.*\\.(c|cc|cp|cpp|cxx|m|mm)$")
      set(GROUP "Source Files${GROUP}")
    # matches: *.h / *.hh / *.hpp / *.hxx
    elseif("${FILE}" MATCHES "^.*\\.(h|hh|hpp|hxx)$")
      set(GROUP "Header Files${GROUP}")
    # matches: everything else
    else()
      set(GROUP "Various Files${GROUP}")
    endif()
    
    # DEBUG
    # message(STATUS "CURRENT_FILE: ${FILE} CURRENT_GROUP: ${GROUP}")
    
    source_group("${GROUP}" FILES "${FILE}")
    list(APPEND FILE_LIST ${FILE})
  endforeach()
  
  # remove unneeded duplicates from the include directory list
  # list(REMOVE_DUPLICATES INCLUDE_DIR_LIST)
  
  set(${OUTPUT_FILE_LIST} "${FILE_LIST}" PARENT_SCOPE)
  set(${OUTPUT_INCLUDE_DIR_LIST} "${INCLUDE_DIR_LIST}" PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# print_cmake_variables
# -----------------------------------------------------------------------------
function(print_cmake_variables)
  get_cmake_property(VARIABLE_NAMES VARIABLES)
  foreach(VARIABLE_NAME ${VARIABLE_NAMES})
    message(STATUS "${VARIABLE_NAME}=${${VARIABLE_NAME}}")
  endforeach()
endfunction()

# -----------------------------------------------------------------------------
# print_cmake_variables
# -----------------------------------------------------------------------------
function(print_cxx_known_features)
  message("Out of all possible C++ compiler features CMake knows about:")
  foreach(FEATURE ${CMAKE_CXX_KNOWN_FEATURES})
    message("  ${FEATURE}")
  endforeach()
  message("")
  message("your C++ compiler supports these features")
  foreach(FEATURE ${CMAKE_CXX_COMPILE_FEATURES})
    message("  ${FEATURE}")
  endforeach()
endfunction()

# -----------------------------------------------------------------------------
# check_configuration_generator
# -----------------------------------------------------------------------------
function(check_configuration_generator)
  if(NOT CMAKE_CONFIGURATION_TYPES) # single-configuration generator...
    if(NOT CMAKE_BUILD_TYPE) # ...with no build type given
      set(CMAKE_BUILD_TYPE "RelWithDebInfo"
        CACHE STRING
        "Options are Debug, Release, RelWithDebInfo or MinSizeRel."
        FORCE
      )
    endif()
    message(STATUS "Single-Configuration Generator > CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
  else()
    message(STATUS "Multi-Configuration Generator > CMAKE_CONFIGURATION_TYPES: ${CMAKE_CONFIGURATION_TYPES}")
  endif()
endfunction()
