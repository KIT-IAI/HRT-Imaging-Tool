# CMakeList.txt: CMake project for "ImageIO".
# Include the sources and define project-specific logic here.
#

# Specify build target for the library
add_library (ImageIO STATIC)

# Define list of project source files
set(SRC_FILES
    ImageExporter.cpp
    ImageImporter.cpp
    ImageIOException.cpp
    )

# Define list of project header files
set(HDR_FILES
    IIO_Defines.h
    ImageExporter.h
    ImageImporter.h
    ImageIOException.h
    )

# Specify precompiled headers
target_precompile_headers(ImageIO PRIVATE stdafx.h)

# Specify source and header files
target_sources(ImageIO PRIVATE ${SRC_FILES})
target_sources(ImageIO PRIVATE ${HDR_FILES})

# link required libraries
target_link_libraries(ImageIO PRIVATE JPEG::JPEG)
target_link_libraries(ImageIO PRIVATE PNG::PNG)
target_link_libraries(ImageIO PRIVATE TIFF::TIFF)
