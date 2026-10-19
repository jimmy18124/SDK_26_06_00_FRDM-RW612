# Add set(CONFIG_USE_middleware_psatest_ns_disable_crypto_cipher_module true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

  if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DTF_M_DISABLE_CRYPTO_CIPHER_MODULE_TESTS
              )
  
  
  endif()

