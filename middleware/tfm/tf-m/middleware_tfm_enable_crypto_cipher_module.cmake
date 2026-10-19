# Add set(CONFIG_USE_middleware_tfm_enable_crypto_cipher_module true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

  if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DPSA_WANT_ALG_CBC_NO_PADDING=1
                        -DPSA_WANT_ALG_CMAC=1
                        -DPSA_WANT_ALG_DETERMINISTIC_ECDSA=1
                        -DPSA_WANT_KEY_TYPE_ECC_KEY_PAIR=1
              )
  
  
  endif()

