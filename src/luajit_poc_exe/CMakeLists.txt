SET( SOURCES 
    main_exe.cpp
   )

add_executable( luajit_poc_exe ${SOURCES}
)

target_link_libraries(luajit_poc_exe luajit_poc_lib )

