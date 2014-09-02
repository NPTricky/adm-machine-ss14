# -----------------------------------------------------------------------------
# Eigen
# -----------------------------------------------------------------------------

get_library_dir_from_current_source_dir(Eigen_LIBRARY_DIR)

add_library(Eigen INTERFACE)
target_include_directories(Eigen INTERFACE
  $<BUILD_INTERFACE:${Eigen_LIBRARY_DIR}>
  $<INSTALL_INTERFACE:include/Eigen>
)

target_compile_definitions(Eigen INTERFACE "EIGEN_NO_DEBUG")

#EIGEN_USE_BLAS
#EIGEN_USE_LAPACKE_STRICT
