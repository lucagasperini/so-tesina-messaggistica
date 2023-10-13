project(tesina-messaggistica)

include_directories(lib/include/)

add_library(matrix SHARED
        lib/src/net.c
        lib/src/log.c 
        lib/src/sys.c
        lib/src/file.c
        lib/src/proto.c
        lib/src/msg.c
        lib/src/users.c
        lib/src/stack.c
        lib/src/dir.c
        lib/src/mem.c
        lib/src/darray.c
)

add_executable(client 
        client/main.c 
)

set(client_dir "\"$ENV{HOME}/.matrix-client\"")

target_compile_definitions(client PRIVATE -DCLIENT_MAIN_DIR=${client_dir})

add_executable(server
        server/request.c
        server/threads.c
        server/main.c
)

set(server_dir "\"$ENV{HOME}/.matrix-server\"")
target_compile_definitions(server PRIVATE -DTHREAD_NUM=10 -DMAX_CONNECTION=10 -DSERVER_MAIN_DIR=${server_dir})

target_link_libraries(server matrix)
target_link_libraries(client matrix)