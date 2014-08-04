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
  message(STATUS "Available Thread(s): ${PROCESSOR_COUNT}")
	set(${OUTPUT_PROCESSOR_COUNT} "${PROCESSOR_COUNT}" PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# check_compiler
# -----------------------------------------------------------------------------
#
# OUTPUT
# - FATAL_ERROR on unsupported c++11
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
  message(STATUS "Available C++ Support: C++11")
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
# - (name) OUTPUT_INCLUDE_FILES
# - (name) OUTPUT_INCLUDE_DIR
#
# OUTPUT
# - OUTPUT_INCLUDE_FILES
#     the list of files extracted recursively from the folder
# - OUTPUT_INCLUDE_DIR
#     the list of directories containing *.h files
#
# -----------------------------------------------------------------------------
function(glob_source_group FOLDER OUTPUT_INCLUDE_FILES OUTPUT_INCLUDE_DIR)
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
  
  set(${OUTPUT_INCLUDE_FILES} "${FILE_LIST}" PARENT_SCOPE)
  set(${OUTPUT_INCLUDE_DIR} "${INCLUDE_DIR_LIST}" PARENT_SCOPE)
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
# print_cxx_known_features
# -----------------------------------------------------------------------------
function(print_cxx_known_features)
  message("CMAKE_CXX_KNOWN_FEATURES:")
  foreach(FEATURE ${CMAKE_CXX_KNOWN_FEATURES})
    message(" - ${FEATURE}")
  endforeach()
  message("CMAKE_CXX_COMPILE_FEATURES:")
  foreach(FEATURE ${CMAKE_CXX_COMPILE_FEATURES})
    message(" - ${FEATURE}")
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

# -----------------------------------------------------------------------------
# check_source_build
# -----------------------------------------------------------------------------
#
# OUTPUT
# - FATAL_ERROR on in-source build
#
# -----------------------------------------------------------------------------
function(check_source_build)
  if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt.")
  endif()
endfunction()

# -----------------------------------------------------------------------------
# string_first_char_to_upper_case
# -----------------------------------------------------------------------------
#
# INPUT
# - INPUT_STRING
#     string to make first character upper case
# - (name) OUTPUT_STRING
#
# OUTPUT
# - OUTPUT_STRING
#     string with first character in upper case
#
# -----------------------------------------------------------------------------
function(string_first_char_to_upper_case INPUT_STRING OUTPUT_STRING)
  string(SUBSTRING "${INPUT_STRING}" 0  1 FIRST)
  string(SUBSTRING "${INPUT_STRING}" 1 -1 TAIL )
  string(TOUPPER "${FIRST}" FIRST)
  set(${OUTPUT_STRING} "${FIRST}${TAIL}" PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# string_underscore_to_camel_case
# -----------------------------------------------------------------------------
#
# INPUT
# - INPUT_STRING
#     underscore separated string about to be in camel case
# - (name) OUTPUT_STRING
#
# OUTPUT
# - OUTPUT_STRING
#     camel case string
#
# -----------------------------------------------------------------------------
function(string_underscore_to_camel_case INPUT_STRING OUTPUT_STRING)
  string(REPLACE "_" ";" INPUT_STRING_LIST ${INPUT_STRING})
  foreach(SUBSTRING ${INPUT_STRING_LIST})
    string_first_char_to_upper_case(SUBSTRING SUBSTRING_UPPER)
    set(CAMEL_CASE ${CAMEL_CASE}${SUBSTRING_UPPER})
  endforeach()
  set(${OUTPUT_STRING} ${CAMEL_CASE} PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# get_library_dir_from_current_source_dir
# -----------------------------------------------------------------------------
#
# INPUT
# - (name) OUTPUT_PATH
#
# OUTPUT
# - OUTPUT_PATH
#     path to the respective library
#
# -----------------------------------------------------------------------------
function(get_library_dir_from_current_source_dir OUTPUT_PATH)
  # retrieve library name from directory name
  get_filename_component(CURRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
  string(REGEX REPLACE "[.].*$" "" CURRENT_LIBRARY_NAME "${CURRENT_DIR}")
  # retrieve library directory from library name
  get_filename_component(PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
  set(CURRENT_LIBRARY_DIR "${PARENT_DIR}/${CURRENT_LIBRARY_NAME}")
  set(${OUTPUT_PATH} ${CURRENT_LIBRARY_DIR} PARENT_SCOPE)
endfunction()
