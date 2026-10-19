# Add set(CONFIG_USE_driver_conn_fwloader_fw_bin true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/rw61x/rw61x_cpu1.c
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/rw61x/rw61x_cpu2.c
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/iw610x/iw610_cpu12_se.c
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/iw610x/iw610_cpu2_se.c
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/iw610x/iw610_cpu1_se.c
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/8987/8987_cpu12.c
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/8987/8987_cpu1.c
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/8987/8987_cpu2.c
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/IW416/IW416_cpu12.c
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/IW416/IW416_cpu1.c
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/IW416/IW416_cpu2.c
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/nw61x/nw61x_cpu12_se.c
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/nw61x/nw61x_cpu1_se.c
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/nw61x/nw61x_cpu2_se.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/inc/rw61x
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/rw61x
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/inc/iw610x
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/iw610x
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/inc/8987
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/8987
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/inc/IW416
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/IW416
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/inc/nw61x
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/nw61x
          ${CMAKE_CURRENT_LIST_DIR}/fw_bin/script
        )

  
