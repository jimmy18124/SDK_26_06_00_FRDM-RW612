# Add set(CONFIG_USE_component_zephyr_headers_kernel true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/zephyr/include
          ${CMAKE_CURRENT_LIST_DIR}/zephyr/misc/generated
          ${CMAKE_CURRENT_LIST_DIR}/zephyr/misc
        )

  
