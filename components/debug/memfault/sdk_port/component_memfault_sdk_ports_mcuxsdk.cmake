# Add set(CONFIG_USE_component_memfault_sdk_ports_mcuxsdk true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/src/memfault_platform_flash_backed_coredump.c
          ${CMAKE_CURRENT_LIST_DIR}/src/memfault_platform_port.c
          ${CMAKE_CURRENT_LIST_DIR}/include/memfault_metrics_heartbeat_config.def
          ${CMAKE_CURRENT_LIST_DIR}/include/memfault_trace_reason_user_config.def
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/include
        )

  
