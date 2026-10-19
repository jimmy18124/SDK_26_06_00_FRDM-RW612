# Add set(CONFIG_USE_middleware_tfm_s_isolation_1_cfg true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

  if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DTFM_ISOLATION_LEVEL=1
                        -DCONFIG_TFM_CONNECTION_POOL_ENABLE
                        -DCONFIG_TFM_SPM_BACKEND_SFN=1
              )
  
  
  endif()

