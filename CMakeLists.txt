cmake_minimum_required(VERSION 3.14)
project(couchbase C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O0 -Wall -std=gnu99 -fvisibility=hidden")

get_filename_component(PHP_SRC_PATH "~/code/php/php-src" ABSOLUTE)

include_directories(
        "${PHP_SRC_PATH}"
        "${PHP_SRC_PATH}/main"
        "${PHP_SRC_PATH}/Zend"
        "${PHP_SRC_PATH}/TSRM"
        "${PHP_SRC_PATH}/ext"
        "${PHP_SRC_PATH}/sapi"
        "${PROJECT_SOURCE_DIR}"
)

file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/**/**/*.c ${PROJECT_SOURCE_DIR}/src/**/*.c ${PROJECT_SOURCE_DIR}/*.c)
list(FILTER SOURCE_FILES EXCLUDE REGEX "crypto.c")

add_library(couchbase ${SOURCE_FILES})
