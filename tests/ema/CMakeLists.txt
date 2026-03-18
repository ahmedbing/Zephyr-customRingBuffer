cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ema_tests)

target_sources(app PRIVATE
    src/test_ema.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/ema.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)