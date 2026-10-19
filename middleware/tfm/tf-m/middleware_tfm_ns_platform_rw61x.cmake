# Add set(CONFIG_USE_middleware_tfm_ns_platform_rw61x true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/common/CMSIS_Driver/Driver_USART.c
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/plat_iak_pk/plat_iak_pk.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/ext/target/nxp/rw61x/plat_iak_pk
        )

  
