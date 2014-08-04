# can't guarantee cmake backwards compatibility in such a small project
cmake_minimum_required(VERSION 3.0.0)

project(Mate)

# guarantee the 3.0.0 feature set in later cmake releases...
if(CMAKE_VERSION VERSION_GREATER 3.0.0)
  message(STATUS "cmake_policy set to 3.0.0 for backwards compatibility")
  cmake_policy(VERSION 3.0.0)
endif()

# search modules in own directory first
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# -----------------------------------------------------------------------------
# Include CMake Macros & Functions
# -----------------------------------------------------------------------------

include(Utility)

# -----------------------------------------------------------------------------
# Global Variables
# -----------------------------------------------------------------------------

check_source_build()
check_configuration_generator()
check_compiler()
get_processor_count(HOST_PROCESSOR_COUNT)

# installation related
set(CUSTOM_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
set(CUSTOM_INSTALL_LIBRARY_DIR "${CUSTOM_INSTALL_PREFIX}/lib")

# cmake related
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_DEBUG_POSTFIX "d") # common debug library & executable postfix
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON) # internal include priority

# project related
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION_TWEAK 0)
set(PROJECT_EXECUTABLE_NAME "${PROJECT_NAME}")
set(PROJECT_CONTACT "GitHub User <@NPTricky>")
set(PROJECT_HOMEPAGE "https://github.com/NPTricky/adm-machine-ss14")

# bit variable
set(BIT 32) # BIT 32 = MATCHES "4"
if(${CMAKE_SIZEOF_VOID_P} MATCHES "8") # BIT 64
  set(BIT 64)
  set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS ON)
endif()

# -----------------------------------------------------------------------------
# Settings
# -----------------------------------------------------------------------------

# build
option(BUILD_TESTS "Build Tests ()" OFF)
option(BUILD_DOCS "Build Documentation (Doxygen)" OFF)

mark_as_advanced(
  BUILD_TESTS
)

# configuration
option(CONFIG_USE_SOLUTION_FOLDERS "Enable to group build targets into folders in some IDE's" ON)

# property configuration
set_property(GLOBAL PROPERTY USE_FOLDERS ${CONFIG_USE_SOLUTION_FOLDERS}) 

# -----------------------------------------------------------------------------
# Third Party Libraries
# -----------------------------------------------------------------------------

list(APPEND THIRD_PARTY_LIBRARIES
  eigen
)

foreach(THIRD_PARTY_LIBRARY ${THIRD_PARTY_LIBRARIES})
  add_subdirectory(ext/${THIRD_PARTY_LIBRARY}.cmake)
endforeach()

# -----------------------------------------------------------------------------
# Source
# -----------------------------------------------------------------------------

glob_source_group(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  Mate_INCLUDE_FILES
  Mate_INCLUDE_DIR
)
add_executable(Mate
  ${Mate_INCLUDE_FILES}
)
set_property(TARGET Mate
  PROPERTY INCLUDE_DIRECTORIES
    ${Mate_INCLUDE_DIR}
)
target_link_libraries(Mate Eigen)

#set_property(TARGET Mate
#  PROPERTY DEFINE_SYMBOL "BUILD_DLL"
#)
#set_property(TARGET Mate
#  PROPERTY COMPILE_DEFINITIONS ${Eigen_COMPILE_DEFINITIONS}
#)

# -----------------------------------------------------------------------------
# Testing
# -----------------------------------------------------------------------------

if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# -----------------------------------------------------------------------------
# Documentation
# -----------------------------------------------------------------------------

if(BUILD_DOCS)
  add_subdirectory(doc)
endif()
