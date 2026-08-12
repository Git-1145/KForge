cmake_minimum_required(VERSION 3.15)
project(KForge CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC 编译/链接选项（对齐原 init_build.bat）
if(MSVC)
    add_compile_options(/O2 /utf-8 /EHsc /MD)
    add_link_options(/SUBSYSTEM:CONSOLE /OPT:REF /OPT:ICF)
endif()

# 项目根目录作为 include 根：所有源文件统一写 #include "base/KF.hpp"
include_directories(${CMAKE_SOURCE_DIR})

# ===== base 静态库 (KF) =====
file(GLOB BASE_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/base/*.cpp)
add_library(KF STATIC ${BASE_SOURCES})
target_include_directories(KF PUBLIC ${CMAKE_SOURCE_DIR})

# ===== 辅助函数：为单个 cpp 创建可执行目标 =====
# exe 输出到源文件所在目录（保持和原 init_build.bat 一致，便于读取 cfg.kson 等相对路径资源）
function(kforge_add_executable src)
    get_filename_component(name ${src} NAME_WE)
    get_filename_component(dir  ${src} DIRECTORY)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE KF)
    # 将 exe 复制到源文件所在目录（VS 多配置生成器会追加 Release/ 等）
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:${name}>"
            "${dir}/$<TARGET_FILE_NAME:${name}>"
        COMMENT "Copying ${name} to ${dir}")
    # study/ 下的文件无 main() 时默认不参与全量构建
    if("${dir}" MATCHES "/study/")
        set_target_properties(${name} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()
endfunction()

# ===== test 可执行文件 =====
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/test/*.cpp)
foreach(src ${TEST_SOURCES})
    kforge_add_executable(${src})
endforeach()

# ===== study 可执行文件（递归） =====
file(GLOB_RECURSE STUDY_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/study/*.cpp)
foreach(src ${STUDY_SOURCES})
    kforge_add_executable(${src})
endforeach()
