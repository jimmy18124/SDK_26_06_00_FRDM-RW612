# Add set(CONFIG_USE_middleware_wireless_framework_dbg_fault_handlers true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/services/DBG/fault_handlers/fwk_fault_handlers_common.c
          ${CMAKE_CURRENT_LIST_DIR}/services/DBG/fault_handlers/fwk_fault_handlers_utils.c
        )

        if(CONFIG_USE_middleware_freertos-kernel)
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/services/DBG/fault_handlers/rtos_port/fwk_fault_handlers_freertos.c
        )
    endif()

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/services/DBG/fault_handlers
          ${CMAKE_CURRENT_LIST_DIR}/services/DBG/fault_handlers/rtos_port
        )

  
