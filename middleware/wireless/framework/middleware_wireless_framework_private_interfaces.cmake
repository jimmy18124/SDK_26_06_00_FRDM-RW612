# Add set(CONFIG_USE_middleware_wireless_framework_private_interfaces true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/Common
          ${CMAKE_CURRENT_LIST_DIR}/platform/include
          ${CMAKE_CURRENT_LIST_DIR}/services/DBG
          ${CMAKE_CURRENT_LIST_DIR}/services/FactoryDataProvider
          ${CMAKE_CURRENT_LIST_DIR}/services/FileSystem
          ${CMAKE_CURRENT_LIST_DIR}/services/FSCI/Interface
          ${CMAKE_CURRENT_LIST_DIR}/services/FunctionLib
          ${CMAKE_CURRENT_LIST_DIR}/services/HWParameter
          ${CMAKE_CURRENT_LIST_DIR}/services/LowPower
          ${CMAKE_CURRENT_LIST_DIR}/services/ModuleInfo
          ${CMAKE_CURRENT_LIST_DIR}/services/NVM/Interface
          ${CMAKE_CURRENT_LIST_DIR}/services/NVM/Source
          ${CMAKE_CURRENT_LIST_DIR}/services/OtaSupport/Interface
          ${CMAKE_CURRENT_LIST_DIR}/services/OTW/Interface
          ${CMAKE_CURRENT_LIST_DIR}/services/PDM/Include
          ${CMAKE_CURRENT_LIST_DIR}/services/PDUM/Include
          ${CMAKE_CURRENT_LIST_DIR}/services/SecLib_RNG
          ${CMAKE_CURRENT_LIST_DIR}/services/Sensors
          ${CMAKE_CURRENT_LIST_DIR}/services/SFC
          ${CMAKE_CURRENT_LIST_DIR}/../XCVR/drv
          ${CMAKE_CURRENT_LIST_DIR}/../XCVR/drv/nb2p4ghz
        )

        if((CONFIG_DEVICE_ID STREQUAL RW610 OR CONFIG_DEVICE_ID STREQUAL RW612))
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/platform/rw61x
        )
    endif()

  
