cmake_minimum_required(VERSION 3.17)
project(akuna)

set(CMAKE_CXX_STANDARD 20)

file(GLOB HEADER_FILES book/*.hpp book/*.inl)

file(GLOB SOURCE_FILES book/*.cpp *.cpp)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

set(DATA_PATH "${CMAKE_BINARY_DIR}")

file(MAKE_DIRECTORY ${DATA_PATH})

file(GLOB files "*.csv")
foreach (file ${files})
    configure_file(${file} ${DATA_PATH} COPYONLY)
endforeach ()