# Copyright 2024-2026 NXP
# SPDX-License-Identifier: BSD-3-Clause

if(${CONFIG_OT})
    message(STATUS "CONFIG_OT=${CONFIG_OT}")
    
    # ========== Platform Detection ==========
    if(NOT DEFINED COEX_PLATFORM)
        # Auto-detect platform based on board name
        if(${board} MATCHES "^(frdmrw612|rdrw612bga)$")
            set(COEX_PLATFORM "rw612")
        elseif(${board} MATCHES "^evkcmimxrt1060$")
            set(COEX_PLATFORM "rt1060")
        elseif(${board} MATCHES "^(evkmimxrt1170|evkbmimxrt1170)$")
            set(COEX_PLATFORM "rt1170")
        else()
            message(FATAL_ERROR "Unsupported board: ${board}. Please define COEX_PLATFORM manually.")
        endif()
    endif()
    
    message(STATUS "COEX_PLATFORM: ${COEX_PLATFORM}")
    
    # ========== Set OpenThread Source Directory ==========
    if(EXISTS ${SdkRootDirPath}/SW-Content-Register.txt)
        set(SDK_TYPE "PACKAGE")
        set(OT_SRC_DIR ${ProjDirPath}/../ot-nxp)
    else()
        set(SDK_TYPE "INTERNAL")
        set(OT_SRC_DIR ${SdkRootDirPath}/middleware/wireless/coex/third_party/ot-nxp)
    endif()
    
    message(STATUS "SDK_TYPE: ${SDK_TYPE}")
    message(STATUS "OT_SRC_DIR: ${OT_SRC_DIR}")
    
    # ========== Include Platform-Specific Configuration ==========
    set(PLATFORM_CONFIG_FILE ${CMAKE_CURRENT_LIST_DIR}/platform/${COEX_PLATFORM}/ot_config.cmake)
    if(EXISTS ${PLATFORM_CONFIG_FILE})
        include(${PLATFORM_CONFIG_FILE})
    else()
        message(FATAL_ERROR "Platform configuration not found: ${PLATFORM_CONFIG_FILE}")
    endif()
    
    # ========== Common OpenThread Configuration ==========
    # Common compile definitions
    target_compile_definitions(${MCUX_SDK_PROJECT_NAME}
        PUBLIC
        OPENTHREAD_CONFIG_FILE="openthread-core-${COEX_PLATFORM}-config.h"
        OPENTHREAD_PROJECT_CORE_CONFIG_FILE="openthread-core-${COEX_PLATFORM}-config.h"
        OPENTHREAD_CORE_CONFIG_PLATFORM_CHECK_FILE="openthread-core-${COEX_PLATFORM}-config-check.h"
        OPENTHREAD_PROJECT_LIB_CONFIG_FILE="openthread-core-${COEX_PLATFORM}-config.h"
    )
    
    # Common include directories
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
        ${OT_SRC_DIR}/src/common
        ${OT_SRC_DIR}/openthread/src
        ${OT_SRC_DIR}/openthread/include
        ${OT_SRC_DIR}/openthread/src/core
        ${OT_SRC_DIR}/openthread/examples/platforms
        ${OT_SRC_DIR}/openthread/third_party/mbedtls
        ${OT_SRC_DIR}/third_party/mbedtls/configs
    )
endif()
