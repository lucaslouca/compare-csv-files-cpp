ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-Wall")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE debug)
ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-std=c++20")

# Find the packages we need.
find_package(benchmark REQUIRED)

# Add the list of include paths to be used to search for include files
#include_directories(/usr/local/include)
#include_directories(/opt/homebrew/include)

# Search all the .cpp files in the directory where CMakeLists lies and set them to ${SOURCE_FILES}
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp) 

# Search all the .h files in the directory where CMakeLists lies and set them to ${INCLUDE_FILES}
file(GLOB INCLUDE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

# Add the executable Example to be built from the source files
add_executable(compapp ${SOURCE_FILES} ${INCLUDE_FILES})

# Add extra lib directories from the library folder
#link_directories(/usr/local/lib)
#link_directories(/opt/homebrew/lib)

# Link the executable to the libraries.
target_link_libraries(compapp benchmark::benchmark)
target_link_libraries(compapp benchmark::benchmark_main)