# Add set(CONFIG_USE_middleware_tfm_board_headers true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      if(CONFIG_BOARD STREQUAL mcimx93evk)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/mcimx93evk
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/mcimx93evk/drivers/trdc
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/mcimx93evk/partition
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/mcimx93evk/Device/Include
        )
    endif()

        if(CONFIG_BOARD STREQUAL mcxn5xxevk)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/mcxn5xxevk
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/mcxn5xxevk/partition
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/mcxn5xxevk/Device/Include
        )
    endif()

        if(CONFIG_BOARD STREQUAL frdmrw612)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmrw612
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmrw612/partition
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmrw612/Device/Include
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/platform/include
        )
    endif()

        if(CONFIG_BOARD STREQUAL rdrw612bga)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rdrw612bga
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rdrw612bga/partition
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rdrw612bga/Device/Include
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/platform/include
        )
    endif()

        if(CONFIG_BOARD STREQUAL frdmmcxa577)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxa577
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxa577/partition
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxa577/Device/Include
        )
    endif()

        if(CONFIG_BOARD STREQUAL frdmmcxl255)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxl255
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxl255/partition
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxl255/Device/Include
        )
    endif()

        if(CONFIG_BOARD STREQUAL mcxn9xxevk)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/mcxn9xxevk
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/mcxn9xxevk/partition
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/mcxn9xxevk/Device/Include
        )
    endif()

        if(CONFIG_BOARD STREQUAL frdmmcxn947)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxn947
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxn947/partition
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxn947/Device/Include
        )
    endif()

        if(CONFIG_BOARD STREQUAL mimxrt700evk)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/mimxrt700evk
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/mimxrt700evk/partition
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/mimxrt700evk/Device/Include
        )
    endif()

        if(CONFIG_BOARD STREQUAL frdmmcxa287)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxa287
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxa287/partition
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxa287/Device/Include
        )
    endif()

        if(CONFIG_BOARD STREQUAL frdmmcxn236)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxn236
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxn236/partition
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/frdmmcxn236/Device/Include
        )
    endif()

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DTARGET_CONFIG_HEADER_FILE="config_tfm_target.h"
              )
  
  
  endif()

