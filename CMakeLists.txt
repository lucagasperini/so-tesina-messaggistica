project(tesina-messaggistica)

cmake_minimum_required(VERSION 3.11)

if(NOT CMAKE_BINARY_DIR)
        set(PROJECT_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

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

target_link_libraries(matrix PRIVATE Threads::Threads)

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
# Add -DMATRIX_BIG_ENDIAN if compiled for BIG ENDIAN ARCH
target_compile_definitions(server PRIVATE -DTHREAD_NUM=10 -DMAX_CONNECTION=10 -DSERVER_MAIN_DIR=${server_dir})

target_link_libraries(server matrix Threads::Threads)
target_link_libraries(client matrix Threads::Threads)