include (GenerateExportHeader)

include_directories( ../../thirdparty/luajit-rocks/luajit-2.1/src )

# This bypass the need to setup the correct working directory when running the executable
add_definitions( "-DLUAJIT_POC_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"" )

SET( SOURCES 
    main.cpp main.hpp
   )

add_library( luajit_poc_lib SHARED ${SOURCES}
)

target_link_libraries(luajit_poc_lib libluajit )


set_target_properties(luajit_poc_lib PROPERTIES OUTPUT_NAME luajit_poc )

# generate standard dll import/export macro header
GENERATE_EXPORT_HEADER( luajit_poc_lib
             BASE_NAME LuaJitPoc
             EXPORT_MACRO_NAME LUAJITPOC_API
             EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/luajitpoc_export.h
             STATIC_DEFINE LuaJitPoc_BUILT_AS_STATIC
)

# Project depending on luajit_poc_lib will automatically get this include dir
target_include_directories(luajit_poc_lib PUBLIC ${CMAKE_BINARY_DIR}/exports)