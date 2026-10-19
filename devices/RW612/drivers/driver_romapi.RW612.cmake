# Add set(CONFIG_USE_driver_romapi true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/romapi/bootloader/fsl_romapi.c
          ${CMAKE_CURRENT_LIST_DIR}/romapi/flexspi/fsl_romapi_flexspi.c
          ${CMAKE_CURRENT_LIST_DIR}/romapi/iap/fsl_romapi_iap.c
          ${CMAKE_CURRENT_LIST_DIR}/romapi/nboot/fsl_romapi_nboot.c
          ${CMAKE_CURRENT_LIST_DIR}/romapi/otp/fsl_romapi_otp.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/romapi/bootloader
          ${CMAKE_CURRENT_LIST_DIR}/romapi/flexspi
          ${CMAKE_CURRENT_LIST_DIR}/romapi/iap
          ${CMAKE_CURRENT_LIST_DIR}/romapi/nboot
          ${CMAKE_CURRENT_LIST_DIR}/romapi/otp
        )

  
