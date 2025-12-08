# CMakeList.txt: CMake project for "Utilities".
# Include the sources and define project-specific logic here.
#

# Specify build target for the library
add_library (Utilities STATIC)

# Define list of project source files
set(SRC_FILES
    AbstractMatrix.cpp
    AbstractVector.cpp
    Compare.cpp
    DateTimeUtilities.cpp
    DenseMatrix.cpp
    DenseVector.cpp
    EnvironmentVariable.cpp
    FileUtilities.cpp
    Graph.cpp
    InsructionSet.cpp
    Log.cpp
    MathTools.cpp
    Point.cpp
    Regex.cpp
    SettingsStore.cpp
    SimpleReadWriteLock.cpp
    SLESolver.cpp
    SparseMatrix.cpp
    SparseVector.cpp
    SQLiteDatabase.cpp
    SQLiteException.cpp
    SQLiteTable.cpp
    SQLiteTableRow.cpp
    StringUtilities.cpp
    SynchronizedGraph.cpp
    TextFileOutput.cpp
    TimeMeasurement.cpp
    Utilities.cpp
    )

# Define list of project header files
set(HDR_FILES
    AbstractMatrix.h
    AbstractVector.h
    ArrayUtilities.h
    Cancelable.h
    Compare.h
    Computable.h
    DateTimeUtilities.h
    DenseMatrix.h
    DenseVector.h
    EnvironmentVariable.h
    FileUtilities.h
    Graph.h
    InstructionSet.h
    Log.h
    MathTools.h
    Point.h
    Progress.h
    ProgressForwarder.h
    Progressing.h
    ProgressListener.h
    Regex.h
    SettingsStore.h
    SimpleReadWriteLock.h
    SLESolver.h
    SparseMatrix.h
    SparseVector.h
    SQLiteDatabase.h
    SQLiteException.h
    SQLiteTable.h
    SQLiteTableRow.h
    StringUtilities.h
    SynchronizedGraph.h
    TextFileOutput.h
    TimeMeasurement.h
    Utilities.h
    Vectors.h
    Win32HeapRAII.h
    )

# Define list of project include folders
set(HDR_FOLDERS
    ${PROJECT_SOURCE_DIR}/alglib
    )

# The following line can provide the memcpy_s() and some other ..._s() functions
# on some systems, but apparently Linux systems are generally not among them
# target_compile_definitions(Utilities PRIVATE __STDC_WANT_LIB_EXT1__=1)

# Specify precompiled headers
target_precompile_headers(Utilities PRIVATE stdafx.h)

# Specify source and header files
target_sources(Utilities PRIVATE ${SRC_FILES})
target_sources(Utilities PRIVATE ${HDR_FILES})

# Specify include directories
target_include_directories(Utilities PRIVATE ${HDR_FOLDERS})

# link required libraries
target_link_libraries(Utilities PRIVATE Boost::asio Boost::log Boost::process Boost::regex)
target_link_libraries(Utilities PRIVATE unofficial::sqlite3::sqlite3)
target_link_libraries(Utilities PRIVATE alglib)
