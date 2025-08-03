cmake_minimum_required(VERSION 3.22)
project(fq-hll-bot VERSION 1.0 DESCRIPTION "A discord bot with FQ-HLL")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Add executable
file(GLOB_RECURSE FQHLL_SRC
    FQ-HyperLogLog-Autocorrect/fq_hll_cpp/src/src/*.cpp
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/env_parser/env_parser.cpp
    src/utils/utils.cpp
    ${FQHLL_SRC}
)

# DPP linkage
find_package(DPP REQUIRED)
target_link_libraries(${PROJECT_NAME} ${DPP_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${DPP_INCLUDE_DIR})

# FQ-HLL includes
target_include_directories(${PROJECT_NAME} PRIVATE 
    FQ-HyperLogLog-Autocorrect/fq_hll_cpp/src/include
    src/env_parser
    src/utils
)

# Set C++ standard
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

set(CMAKE_CXX_FLAGS_RELEASE "-O2")