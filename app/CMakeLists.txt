cmake_minimum_required(VERSION 3.20.0)
set(BOARD kubot_f765vi)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app)

target_sources(app PRIVATE src/main.c)
