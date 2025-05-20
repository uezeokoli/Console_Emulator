cmake_minimum_required(VERSION 3.22.1)

project(Banana LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Add SDL2 include directories
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
    
    src/main.cpp
    src/BananaConsole.cpp
    src/BananaCpu.cpp
    src/BananaMemory.cpp
    src/BananaGpu.cpp
    
)

# Link against SDL2
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
