cmake_minimum_required(VERSION 3.14)
project(BucketTest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(BUCKET_SRC
    ${CMAKE_SOURCE_DIR}/../src/bucket.cpp
    ${CMAKE_SOURCE_DIR}/../src/queue.cpp
    ${CMAKE_SOURCE_DIR}/../src/circular_queue.cpp
    ${CMAKE_SOURCE_DIR}/../src/file_handle.cpp
)

# Try system Catch2 first, fall back to FetchContent
find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
  )
  FetchContent_MakeAvailable(Catch2)
endif()

# Add the executable target
add_executable(bucket.tst test.cpp ${BUCKET_SRC})

# Link against Catch2
target_link_libraries(bucket.tst Catch2::Catch2WithMain)

# Suppress format-security warning from PRINT() variadic template in source headers
target_compile_options(bucket.tst PRIVATE -Wno-format-security)


# Enable testing
enable_testing()

# Add a test
add_test(NAME bucket.tstTest COMMAND bucket.tst)