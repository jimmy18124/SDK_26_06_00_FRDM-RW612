# Add set(CONFIG_USE_middleware_nxp_iot_agent_claimcode true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/ex/src/utils/iot_agent_claimcode_encrypt_els.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/inc
          ${CMAKE_CURRENT_LIST_DIR}/ex/inc
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DNXP_IOT_AGENT_HAVE_HOSTCRYPTO_MBEDTLS=1
                        -DNXP_IOT_AGENT_HAVE_PSA=1
              )
  
  
  endif()

