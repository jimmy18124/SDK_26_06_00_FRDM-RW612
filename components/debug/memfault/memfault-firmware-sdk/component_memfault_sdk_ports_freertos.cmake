# Add set(CONFIG_USE_component_memfault_sdk_ports_freertos true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/ports/freertos/src/memfault_core_freertos.c
          ${CMAKE_CURRENT_LIST_DIR}/ports/freertos/src/memfault_freertos_ram_regions.c
          ${CMAKE_CURRENT_LIST_DIR}/ports/freertos/src/memfault_metrics_freertos.c
          ${CMAKE_CURRENT_LIST_DIR}/ports/freertos/src/memfault_panics_freertos.c
          ${CMAKE_CURRENT_LIST_DIR}/ports/freertos/src/memfault_sdk_metrics_freertos.c
          ${CMAKE_CURRENT_LIST_DIR}/ports/freertos/src/memfault_sdk_metrics_thread.c
          ${CMAKE_CURRENT_LIST_DIR}/ports/freertos/src/memfault_self_test_platform.c
          ${CMAKE_CURRENT_LIST_DIR}/ports/freertos/config/memfault_metrics_heartbeat_freertos_config.def
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/ports/include
          ${CMAKE_CURRENT_LIST_DIR}/.
        )

  
