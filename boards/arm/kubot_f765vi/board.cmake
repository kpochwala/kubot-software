# SPDX-License-Identifier: Apache-2.0

set(BOARD_FLASH_RUNNER blackmagicprobe)
board_runner_args(blackmagicprobe "--gdb-serial=192.168.4.1:2022" "--monitor-frequency=1000000" "--connect-srst")

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/blackmagicprobe.board.cmake)
