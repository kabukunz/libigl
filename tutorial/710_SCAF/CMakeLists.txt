get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

add_executable(${PROJECT_NAME}_bin main.cpp)

if(LIBIGL_WITH_MMG)
  target_link_libraries(${PROJECT_NAME}_bin igl::core igl::opengl igl::opengl_glfw igl::mmg tutorials)
  igl_copy_mmg_dll(${PROJECT_NAME}_bin)
else()
  target_link_libraries(${PROJECT_NAME}_bin igl::core igl::opengl igl::opengl_glfw igl::triangle tutorials)
endif()
