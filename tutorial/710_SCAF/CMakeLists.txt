get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

add_executable(${PROJECT_NAME}_bin main.cpp)

if(LIBIGL_WITH_MMG)
  set(MMG_LIB igl::mmg)
endif()

if(LIBIGL_WITH_TRIANGLE)
  set(TRIANGLE_LIB igl::triangle)
endif()

target_link_libraries(${PROJECT_NAME}_bin igl::core igl::opengl igl::opengl_glfw ${MMG_LIB} ${TRIANGLE_LIB} tutorials)

if(LIBIGL_WITH_MMG)
  igl_copy_mmg_dll(${PROJECT_NAME}_bin)
endif()
