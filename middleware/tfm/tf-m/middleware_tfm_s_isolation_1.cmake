# Add set(CONFIG_USE_middleware_tfm_s_isolation_1 true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/core/backend_sfn.c
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/core/psa_interface_sfn.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/secure_fw/spm/include/ffm
        )

  
