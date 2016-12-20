include_directories ( include )
include_directories ( ${CMAKE_BINARY_DIR}/client )

#include CEF Library
include("${CMAKE_SOURCE_DIR}/client/cef.cmake")

FLATBUFFERS_GENERATE_C_HEADERS ( OXIDE_CLIENT_IDL idl/test.idl )
add_custom_target(oxide_client_flatbuffers DEPENDS ${OXIDE_CLIENT_IDL_OUTPUTS})

set ( OXIDE_SRCS
    src/client_app.cpp
    include/client_app.hpp
    src/client_handler.cpp
    include/client_handler.hpp
)

set ( OXIDE_SRCS_LINUX
    src/linux/cefclient.cpp
    src/linux/client_handler.cpp
)

set ( OXIDE_SRCS_MACOSX
    src/mac/cefclient.cpp
    src/mac/client_handler.cpp
)

set ( OXIDE_SRCS_WINDOWS
    src/win/oxide_client.exe.manifest
    src/win/oxide_client.rc
    include/win/resource.h
    src/win/cefclient.cpp
    src/win/client_handler.cpp
)

APPEND_PLATFORM_SOURCES(OXIDE_SRCS)
source_group(oxide_client FILES ${OXIDE_SRCS})

set ( OXIDE_RES_SRCS_WINDOWS
  res/oxide.ico
  res/small.ico
)
APPEND_PLATFORM_SOURCES(OXIDE_RES_SRCS)
source_group(oxide_client\\\\res FILES ${OXIDE_RES_SRCS})

set(OXIDE_SRCS
  ${OXIDE_SRCS}
  ${OXIDE_RES_SRCS}
)

set(OXIDE_HELPER_SRCS_MACOSX
  src/mac/process_helper.cpp
)
APPEND_PLATFORM_SOURCES(OXIDE_HELPER_SRCS)
source_group(oxide_client FILES ${OXIDE_HELPER_SRCS})

set(OXIDE_RESOURCES_MAC_SRCS_MACOSX
  src/mac/Info.plist
  src/mac/oxide_client.icns
  )
APPEND_PLATFORM_SOURCES(OXIDE_RESOURCES_MAC_SRCS)
source_group(oxide_client\\\\mac FILES ${OXIDE_RESOURCES_MAC_SRCS})

set(OXIDE_RESOURCES_MAC_ENGLISH_LPROJ_SRCS_MACOSX
  src/mac/English.lproj/InfoPlist.strings
  src/mac/English.lproj/MainMenu.xib
  )
APPEND_PLATFORM_SOURCES(OXIDE_RESOURCES_MAC_ENGLISH_LPROJ_SRCS)
source_group(oxide_client\\\\mac\\\\English.lproj FILES ${OXIDE_RESOURCES_MAC_ENGLISH_LPROJ_SRCS})

set(OXIDE_RESOURCES_SRCS
  ${OXIDE_RESOURCES_MAC_SRCS}
  ${OXIDE_RESOURCES_MAC_ENGLISH_LPROJ_SRCS}
)

set(CEF_TARGET "oxide_client")
set(CEF_LIBRARIES ${CEF_LIB} ${CEF_LIB_WRAPPER} )
if(OS_MACOSX)
  set(CEF_HELPER_TARGET "oxide_client Helper")
endif()

# Logical target used to link the libcef library.
#ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

# Determine the target output directory.
SET_CEF_TARGET_OUT_DIR()

#
# Linux configuration.
#

if(OS_LINUX)
  # Executable target.
  add_executable(${CEF_TARGET} ${OXIDE_SRCS})
  add_dependencies(${CEF_TARGET} cef oxide_client_flatbuffers)
  link_directories ()
  target_link_libraries(${CEF_TARGET} ${CEF_LIBRARIES} ${CEF_STANDARD_LIBS})

  # Set rpath so that libraries can be placed next to the executable.
  set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
  set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
  set_target_properties(${CEF_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR})

  # Copy binary and resource files to the target output directory.
  COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")

  # Set SUID permissions on the chrome-sandbox target.
  SET_LINUX_SUID_PERMISSIONS("${CEF_TARGET}" "${CEF_TARGET_OUT_DIR}/chrome-sandbox")
endif()


#
# Mac OS X configuration.
#

if(OS_MACOSX)
  # Output paths for the app bundles.
  set(CEF_APP "${CEF_TARGET_OUT_DIR}/${CEF_TARGET}.app")
  set(CEF_HELPER_APP "${CEF_TARGET_OUT_DIR}/${CEF_HELPER_TARGET}.app")

  # Variable referenced from Info.plist files.
  set(PRODUCT_NAME "${CEF_TARGET}")

  # Helper executable target.
  add_executable(${CEF_HELPER_TARGET} MACOSX_BUNDLE ${OXIDE_HELPER_SRCS})
  add_dependencies(${CEF_HELPER_TARGET} cef oxide_client_flatbuffers)

  #ADD_LOGICAL_TARGET("libcef_dll_wrapper" "${CEF_LIB_WRAPPER_DEBUG}" "${CEF_LIB_WRAPPER_RELEASE}")
  target_link_libraries(${CEF_HELPER_TARGET} ${CEF_LIBRARIES} ${CEF_STANDARD_LIBS})
  set_target_properties(${CEF_HELPER_TARGET} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/mac/helper-Info.plist
    )

  # Fix the framework link in the helper executable.
  FIX_MACOSX_HELPER_FRAMEWORK_LINK(${CEF_HELPER_TARGET} ${CEF_HELPER_APP})

  # Main executable target.
  add_executable(${CEF_TARGET} MACOSX_BUNDLE ${OXIDE_RESOURCES_SRCS} ${OXIDE_SRCS})
  #add_dependencies(${CEF_TARGET} libcef_dll_wrapper "${CEF_HELPER_TARGET}")
  target_link_libraries(${CEF_TARGET} ${CEF_LIBRARIES} ${CEF_STANDARD_LIBS})
  set_target_properties(${CEF_TARGET} PROPERTIES
    RESOURCE "${OXIDE_RESOURCES_SRCS}"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist
    )

  # Copy files into the main app bundle.
  add_custom_command(
    TARGET ${CEF_TARGET}
    POST_BUILD
    # Copy the helper app bundle into the Frameworks directory.
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_HELPER_APP}"
            "${CEF_APP}/Contents/Frameworks/${CEF_HELPER_TARGET}.app"
    # Copy the CEF framework into the Frameworks directory.
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
            "${CEF_APP}/Contents/Frameworks/Chromium Embedded Framework.framework"
    VERBATIM
    )

  # Fix the framework link in the main executable.
  FIX_MACOSX_MAIN_FRAMEWORK_LINK(${CEF_TARGET} ${CEF_APP})

  # Make the other helper app bundles.
  MAKE_MACOSX_HELPERS(${CEF_TARGET} ${CEF_APP})

  if(NOT ${CMAKE_GENERATOR} STREQUAL "Xcode")
    # Manually process and copy over resource files.
    # The Xcode generator handles this via the set_target_properties RESOURCE directive.
    set(PREFIXES "mac/")  # Remove these prefixes from input file paths.
    COPY_MACOSX_RESOURCES("${OXIDE_RESOURCES_SRCS}" "${PREFIXES}" "${CEF_TARGET}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_APP}")
  endif()
endif()


#
# Windows configuration.
#

if(OS_WINDOWS)
  # Executable target.
  add_executable(${CEF_TARGET} WIN32 ${OXIDE_SRCS})
  add_dependencies(${CEF_TARGET} cef oxide_client_flatbuffers)

  ADD_LOGICAL_TARGET("libcef_dll_wrapper" "${CEF_LIB_WRAPPER_DEBUG}" "${CEF_LIB_WRAPPER_RELEASE}")
  target_link_libraries(${CEF_TARGET} ${CEF_LIBRARIES} ${CEF_STANDARD_LIBS})

  if(USE_SANDBOX)
    # Logical target used to link the cef_sandbox library.
    ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
    target_link_libraries(${CEF_TARGET} cef_sandbox_lib ${CEF_SANDBOX_STANDARD_LIBS})
  endif()

  # Add the custom manifest files to the executable.
  #ADD_WINDOWS_MANIFEST("${CEF_TARGET}")

  # Copy binary and resource files to the target output directory.
  COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
endif()