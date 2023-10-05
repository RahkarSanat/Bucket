cmake_minimum_required(VERSION 3.12)
project(BucketTest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(BUCKET_SRC
    ${CMAKE_SOURCE_DIR}/../src/bucket.cpp
    ${CMAKE_SOURCE_DIR}/../src/queue.cpp
    ${CMAKE_SOURCE_DIR}/../src/circular_queue.cpp
    ${CMAKE_SOURCE_DIR}/../src/file_handle.cpp
)

# Locate Catch2 package
find_package(Catch2 REQUIRED)

# Add the executable target
add_executable(bucket.tst test.cpp ${BUCKET_SRC})

# Link against Catch2
target_link_libraries(bucket.tst Catch2::Catch2WithMain)


# Enable testing
enable_testing()

# Add a test
add_test(NAME bucket.tstTest COMMAND bucket.tst)