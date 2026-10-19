# Add set(CONFIG_USE_middleware_nxp_iot_agent_mqtt true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/ex/src/utils/iot_agent_mqtt_freertos.c
          ${CMAKE_CURRENT_LIST_DIR}/src/protobuf/pb_common.c
          ${CMAKE_CURRENT_LIST_DIR}/src/protobuf/pb_decode.c
          ${CMAKE_CURRENT_LIST_DIR}/src/protobuf/ServiceDescriptor.pb.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/inc
          ${CMAKE_CURRENT_LIST_DIR}/ex/inc
          ${CMAKE_CURRENT_LIST_DIR}/src/keystore/psa
          ${CMAKE_CURRENT_LIST_DIR}/src/protobuf
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DPB_FIELD_32BIT
                        -DNXP_IOT_AGENT_HAVE_HOSTCRYPTO_MBEDTLS=1
                        -DNXP_IOT_AGENT_HAVE_PSA=1
                        -DNXP_IOT_AGENT_HAVE_PSA_IMPL_TFM=1
                        -DIOT_AGENT_MQTT_ENABLE=1
              )
  
  
  endif()

