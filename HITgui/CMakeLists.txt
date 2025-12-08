# CMakeList.txt: CMake project for "HITgui".
# Include the sources and define project-specific logic here.
#

# Specify build target for the executable
add_executable(HITgui)

# Define list of project source files
set(SRC_FILES
    HRTImagingToolApp.cpp
    HRTImagingToolDialog.cpp
    HRTImagingToolParameterDialog.cpp
    HRTImagingToolResource.cpp
    )

# Define list of project header files
set(HDR_FILES
    HRTImagingToolApp.h
    HRTImagingToolDialog.h
    HRTImagingToolParameterDialog.h
    HRTImagingToolResource.h
    )

# Define list of project include folders
set(HDR_FOLDERS
    ${PROJECT_SOURCE_DIR}/ImageIO
    ${PROJECT_SOURCE_DIR}/Utilities
    ${PROJECT_SOURCE_DIR}/HITlogic
    )

# Define list of project ressource files
set(XRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/HRTImagingToolResource.xrc
    )

# The following line can provide the memcpy_s() and some other ..._s() functions
# on some systems, but apparently Linux systems are generally not among them
# target_compile_definitions(HITgui PRIVATE __STDC_WANT_LIB_EXT1__=1)

# Compile wxWidgets resource file
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/HRTImagingToolResource.h
        ${CMAKE_CURRENT_SOURCE_DIR}/HRTImagingToolResource.cpp
    COMMAND wxrc ${XRC_FILES} -v -c -e -o ${CMAKE_CURRENT_SOURCE_DIR}/HRTImagingToolResource.cpp
    DEPENDS ${XRC_FILES}
    COMMENT "Compiling XRC resources"
)

# Specify precompiled headers
target_precompile_headers(HITgui PRIVATE pch.h)

# Specify source and header files
target_sources(HITgui PRIVATE ${SRC_FILES})
target_sources(HITgui PRIVATE ${HDR_FILES})

# Specify include directories
target_include_directories(HITgui PRIVATE ${HDR_FOLDERS})

# link required libraries
target_link_libraries(HITgui PRIVATE wx::core wx::base wx::xrc)
target_link_libraries(HITgui PRIVATE HITlogic)
