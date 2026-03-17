cmake_minimum_required(VERSION 3.16)
project(CosmicRayDetector)

set(CMAKE_CXX_STANDARD 17)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
#----------------------------------------------------------------------------
option(WITH_GEANT4_UIVIS "Build with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#----------------------------------------------------------------------------
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# ROOT  (optional but required for histogram output)
# If ROOT is not found the build still works but histograms are disabled.
#----------------------------------------------------------------------------
find_package(ROOT QUIET COMPONENTS Core RIO Hist Tree)
if(ROOT_FOUND)
  message(STATUS "ROOT found: ${ROOT_VERSION} — histograms enabled")
  include(${ROOT_USE_FILE})
  add_definitions(-DHAS_ROOT)
else()
  message(WARNING "ROOT not found — no histogram output will be produced")
endif()

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#----------------------------------------------------------------------------
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#----------------------------------------------------------------------------
add_executable(CosmicRayDetector ${sources} ${headers})
target_link_libraries(CosmicRayDetector ${Geant4_LIBRARIES})
if(ROOT_FOUND)
  target_link_libraries(CosmicRayDetector ${ROOT_LIBRARIES})
endif()

#----------------------------------------------------------------------------
# Copy all scripts to the build directory
#----------------------------------------------------------------------------
set(COSMICRAY_SCRIPTS
  macros/init_vis.mac
  macros/vis.mac
  macros/run.mac
  macros/cosmic_run.mac
  macros/multi_run.mac
  macros/plot_results.C
)

foreach(_script ${COSMICRAY_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
  )
endforeach()

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#----------------------------------------------------------------------------
install(TARGETS CosmicRayDetector DESTINATION bin)
