# Add set(CONFIG_USE_component_els_pkc_platform_rw61x_inf_header_only true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/.
          ${CMAKE_CURRENT_LIST_DIR}/src/platforms/rw61x
          ${CMAKE_CURRENT_LIST_DIR}/src/platforms/rw61x/inc
          ${CMAKE_CURRENT_LIST_DIR}/includes/platform/rw61x
        )

  
