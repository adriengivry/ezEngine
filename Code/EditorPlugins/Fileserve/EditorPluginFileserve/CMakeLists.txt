ez_cmake_init()

ez_requires_qt()

ez_requires(EZ_CMAKE_PLATFORM_WINDOWS)
ez_requires(EZ_3RDPARTY_ENET_SUPPORT)

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(LIBRARY ${PROJECT_NAME})

ez_link_target_qt(TARGET ${PROJECT_NAME} COMPONENTS Core Gui Widgets Network)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  FileservePlugin
  GuiFoundation
)
