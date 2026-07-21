cmake_minimum_required(VERSION 3.16)
if(DEFINED CURRENT_SRC)
    get_filename_component(target_name ${CURRENT_SRC} NAME_WE)
    project(${target_name})

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_executable(${target_name} ${CURRENT_SRC})

    # ========= 如果你使用OpenGL/GLFW/ImGui，可以在这里加链接库 =========
    # find_package(glfw3 REQUIRED)
    # target_link_libraries(${target_name} glfw)
    # ==================================================================
else()
    message(WARNING "未传入 CURRENT_SRC.请使用 VSCode task 启动构建")
endif()
