ez_cmake_init()

ez_requires_editor()

ez_requires_jolt()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(LIBRARY ${PROJECT_NAME})

ez_link_target_qt(TARGET ${PROJECT_NAME} COMPONENTS Core Gui Widgets)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  EditorFramework
  GameEngine
  ModelImporter2
  JoltCooking
)

add_dependencies(${PROJECT_NAME}
  EnginePluginJolt
)


