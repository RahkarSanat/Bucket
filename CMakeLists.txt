cmake_minimum_required(VERSION 3.12)
project(MyProject)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the source files
set(SOURCE_FILES
    main.cpp
    src/bucket.cpp
    src/queue.cpp
    src/circular_queue.cpp
)

# Add an executable target
add_executable(bucket ${SOURCE_FILES})

# Set include directories
target_include_directories(bucket PRIVATE src)

# Set compile options
# target_compile_options(bucket PRIVATE -DTEST_ENABLED)

# Create a custom target for running the executable
add_custom_target(run
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/bucket
    DEPENDS bucket
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Create a custom target for cleaning
add_custom_target(fullclean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}
)