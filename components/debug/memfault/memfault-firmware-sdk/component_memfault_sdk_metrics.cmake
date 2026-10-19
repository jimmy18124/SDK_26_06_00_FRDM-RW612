# Add set(CONFIG_USE_component_memfault_sdk_metrics true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/components/metrics/src/memfault_metrics.c
          ${CMAKE_CURRENT_LIST_DIR}/components/metrics/src/memfault_metrics_battery.c
          ${CMAKE_CURRENT_LIST_DIR}/components/metrics/src/memfault_metrics_connectivity.c
          ${CMAKE_CURRENT_LIST_DIR}/components/metrics/src/memfault_metrics_reliability.c
          ${CMAKE_CURRENT_LIST_DIR}/components/metrics/src/memfault_metrics_serializer.c
          ${CMAKE_CURRENT_LIST_DIR}/components/include/memfault/metrics/heartbeat_config.def
        )

  

