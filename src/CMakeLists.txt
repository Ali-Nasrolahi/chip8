list(APPEND src
    chip8.c
    main.c
)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ../inc)

add_executable(${CMAKE_PROJECT_NAME} ${src})
target_link_libraries(${CMAKE_PROJECT_NAME} ${SDL2_LIBRARIES})
