ez_cmake_init()

ez_requires_editor()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(LIBRARY ${PROJECT_NAME})

ez_link_target_qt(TARGET ${PROJECT_NAME} COMPONENTS Core Gui Widgets Svg)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  EditorFramework
  GameEngine
  SharedPluginScene
)

add_dependencies(${PROJECT_NAME}
  EnginePluginScene
)

ez_copy_plugin_bundle(${PROJECT_NAME} "Scene")
