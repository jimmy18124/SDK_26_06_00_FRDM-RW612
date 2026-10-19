# Copyright 2024-2026 NXP
# SPDX-License-Identifier: BSD-3-Clause

# RW612 OpenThread Platform Configuration
message(STATUS "Loading RW612 OpenThread configuration")

# RW612 specific library directory
set(OT_LIB_DIR ${OT_SRC_DIR}/build_rw612/rw612_ot_cli/lib)
message(STATUS "Build with RW612 ot-cli libs, path=${OT_LIB_DIR}")

# RW612 specific include directories
target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${OT_SRC_DIR}/src/rw/rw612
)

target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
    MBEDTLS_MCUX_ENTROPY=1
)

# Link RW612 OpenThread libraries
TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${OT_LIB_DIR}/libopenthread-cli-ftd.a
    ${OT_LIB_DIR}/libopenthread-cli-ftd.a
    ${OT_LIB_DIR}/libopenthread-ftd.a
    ${OT_LIB_DIR}/libopenthread-hdlc.a
    ${OT_LIB_DIR}/libopenthread-ncp-ftd.a
    ${OT_LIB_DIR}/libopenthread-platform.a
    ${OT_LIB_DIR}/libopenthread-rw612.a
    ${OT_LIB_DIR}/libopenthread-spinel-ncp.a
    ${OT_LIB_DIR}/libot-cli-addons.a
    ${OT_LIB_DIR}/libot-cli-rw612.a
    ${OT_LIB_DIR}/libopenthread-url.a
    ${OT_LIB_DIR}/libopenthread-radio-spinel.a
    ${OT_LIB_DIR}/libtcplp-ftd.a
)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)
