# Add set(CONFIG_USE_middleware_wireless_framework_dbg_nbu_debug_dbg_struct_legacy true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/services/DBG/nbu_dbg/common/legacy
        )

  
