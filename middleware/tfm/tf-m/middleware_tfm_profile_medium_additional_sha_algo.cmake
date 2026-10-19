# Add set(CONFIG_USE_middleware_tfm_profile_medium_additional_sha_algo true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

  if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DPSA_WANT_ALG_SHA_384
                        -DPSA_WANT_ALG_SHA_512
                        -DPSA_WANT_ALG_SHA_512_224
                        -DPSA_WANT_ALG_SHA_512_256
              )
  
  
  endif()

