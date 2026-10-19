# Add set(CONFIG_USE_middleware_tfm_s_partition_load_attest true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/initial_attestation/auto_generated/load_info_tfm_initial_attestation.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/initial_attestation/auto_generated
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/partitions/initial_attestation
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DTFM_PARTITION_INITIAL_ATTESTATION
                        -DATTEST_TOKEN_PROFILE_PSA_IOT_1
                        -DTEST_NS_ATTESTATION
                        -DTEST_S_ATTESTATION
              )
  
  
  endif()

