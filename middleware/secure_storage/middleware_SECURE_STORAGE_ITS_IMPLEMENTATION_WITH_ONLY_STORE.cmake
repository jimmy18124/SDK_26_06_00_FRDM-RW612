# Add set(CONFIG_USE_middleware_SECURE_STORAGE_ITS_IMPLEMENTATION_WITH_ONLY_STORE true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/src/its/implementation_store.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/zephyr/subsys/secure_storage/include
          ${CMAKE_CURRENT_LIST_DIR}/zephyr/subsys/secure_storage/include/internal
          ${CMAKE_CURRENT_LIST_DIR}/include
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DSECURE_STORAGE_ITS_IMPLEMENTATION_WITH_ONLY_STORE
              )
  
  
  endif()

