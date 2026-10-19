# Add set(CONFIG_USE_middleware_edgefast_bluetooth_shell_ble_power_switch true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/source/shell/shell_platform.c
        )

  

