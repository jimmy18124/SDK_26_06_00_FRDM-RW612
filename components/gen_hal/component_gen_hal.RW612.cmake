# Add set(CONFIG_USE_component_gen_hal true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/devices/Wireless/RW/flexspi_clock_setup.c
          ${CMAKE_CURRENT_LIST_DIR}/init.c
          ${CMAKE_CURRENT_LIST_DIR}/zephyr/kernel/device.c
          ${CMAKE_CURRENT_LIST_DIR}/sdk_integ/zephyr_mcux_helper.c
          ${CMAKE_CURRENT_LIST_DIR}/zephyr/misc/empty_file.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/devices/Wireless/RW
          ${CMAKE_CURRENT_LIST_DIR}/_boards/frdmrw612
          ${CMAKE_CURRENT_LIST_DIR}/zephyr/include
          ${CMAKE_CURRENT_LIST_DIR}/zephyr/misc/generated
          ${CMAKE_CURRENT_LIST_DIR}/zephyr/misc
          ${CMAKE_CURRENT_LIST_DIR}/.
          ${CMAKE_CURRENT_LIST_DIR}/_boards/rdrw612bga
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DCONFIG_FLASH_MCUX_FLEXSPI_XIP=1
                        -DCONFIG_XIP=1
              )
  
  
  endif()

