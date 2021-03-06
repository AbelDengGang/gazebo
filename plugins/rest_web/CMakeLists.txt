link_directories(
  ${GAZEBO_LIBRARY_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

set (server_inc
  RestApi.hh
  RestException.hh
  RestWebPlugin.hh
  )


set (server_src
  RestApi.cc
  RestWebPlugin.cc
  )

set (client_inc
  RestUiLoginDialog.hh
  RestUiPlugin.hh
  RestUiWidget.hh
  )


set (client_src
  RestUiLoginDialog.cc
  RestUiPlugin.cc
  RestUiWidget.cc
  )

set (qt_headers
    RestUiWidget.hh
    RestUiLoginDialog.hh
)

add_definitions(${Qt5Core_DEFINITIONS})

include_directories(
  ${GAZEBO_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
)

add_library(RestWebPlugin SHARED ${server_src} )
target_link_libraries(RestWebPlugin ${GAZEBO_libraries} gazebo_msgs)
install (TARGETS RestWebPlugin
         LIBRARY DESTINATION ${GAZEBO_PLUGIN_LIB_INSTALL_DIR}
         ARCHIVE DESTINATION ${GAZEBO_PLUGIN_LIB_INSTALL_DIR}
         RUNTIME DESTINATION ${GAZEBO_PLUGIN_BIN_INSTALL_DIR})
gz_install_includes("plugins" ${server_inc})

add_library(RestUiPlugin SHARED ${client_src} ${headers_MOC})
target_link_libraries(RestUiPlugin ${GAZEBO_libraries} gazebo_msgs)
install (TARGETS RestUiPlugin
         LIBRARY DESTINATION ${GAZEBO_PLUGIN_LIB_INSTALL_DIR}
         ARCHIVE DESTINATION ${GAZEBO_PLUGIN_LIB_INSTALL_DIR}
         RUNTIME DESTINATION ${GAZEBO_PLUGIN_BIN_INSTALL_DIR})
gz_install_includes("plugins" ${client_inc})
