# CMakeList.txt: CMake project for "alglib".
# Include the sources and define project-specific logic here.
#

# Specify build target for the library
add_library (alglib STATIC)

# Define list of project source files
set(SRC_FILES
    alglibinternal.cpp
    alglibmisc.cpp
    ap.cpp
    dataanalysis.cpp
    diffequations.cpp
    fasttransforms.cpp
    integration.cpp
    interpolation.cpp
    linalg.cpp
    optimization.cpp
    solvers.cpp
    specialfunctions.cpp
    statistics.cpp
    )

# Define list of project header files
set(HDR_FILES
    alglibinternal.h
    alglibmisc.h
    ap.h
    dataanalysis.h
    diffequations.h
    fasttransforms.h
    integration.h
    interpolation.h
    linalg.h
    optimization.h
    solvers.h
    specialfunctions.h
    statistics.h
    )

# Specify precompiled headers
target_precompile_headers(alglib PRIVATE stdafx.h)

# Specify source and header files
target_sources(alglib PRIVATE ${SRC_FILES})
target_sources(alglib PRIVATE ${HDR_FILES})
