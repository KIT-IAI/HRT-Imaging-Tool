# CMakeList.txt: CMake top-level project file.
# Specify any global configuration and include sub-projects here.
#

cmake_minimum_required(VERSION 3.22)

# Specify vcpkg settings
if(WIN32)
	set(VCPKG_TARGET_TRIPLET x64-windows-static)
	set(VCPKG_HOST_TRIPLET x64-windows-static)
	set(VCPKG_BOOTSTRAP_OPTIONS -disableMetrics)
else()
	set(VCPKG_TARGET_TRIPLET x64-linux)
	set(VCPKG_HOST_TRIPLET x64-linux)
	set(VCPKG_BOOTSTRAP_OPTIONS -disableMetrics)
endif()

# Specify MKL settings
set(MKL_ROOT ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET})
set(MKL_LINK "static")
set(MKL_INTERFACE "ilp64")
set(MKL_THREADING "sequential")

project(HIT VERSION 1.2.0)

# Specify MSVC build settings
if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	add_compile_definitions(NOMINMAX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost CONFIG REQUIRED COMPONENTS asio log process regex)
find_package(JPEG REQUIRED)
find_package(MKL CONFIG REQUIRED)
find_package(PNG REQUIRED)
find_package(TBB CONFIG REQUIRED)
find_package(TIFF REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(wxWidgets CONFIG REQUIRED)

# Include sub-projects
add_subdirectory(alglib)
add_subdirectory(ImageIO)
add_subdirectory(Utilities)
add_subdirectory(HITlogic)
add_subdirectory(HITgui)
