cmake_minimum_required(VERSION 3.0)

PROJECT(LuaJIT_Perf_POC)

enable_testing()

# Output all libraries and executables in the same directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin
    CACHE PATH "Single directory for all libraries.")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin
    CACHE PATH "Single directory for all executables.")
mark_as_advanced( LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH )

add_subdirectory( src/ )
add_subdirectory( thirdparty/luajit-rocks )

