
include_directories( ../../thirdparty/luajit-rocks/luajit-2.0/src )

# This bypass the need to setup the correct working directory when running the executable
add_definitions( "-DLUAJIT_POC_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"" )

SET( SOURCES 
    main.cpp
   )

add_executable( lutjit_poc_exe ${SOURCES}
)

target_link_libraries(lutjit_poc_exe libluajit )


set_target_properties(lutjit_poc_exe PROPERTIES OUTPUT_NAME lutjit_poc )

