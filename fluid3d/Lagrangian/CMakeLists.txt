
enable_language(C CXX)

# 查找并启用 OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found for Lagrangian3D")
endif()

file(GLOB_RECURSE Lagrangian3D_SOURCE_FILES "./src/*.cpp")
file(GLOB_RECURSE Lagrangian3D_HEADER_FILES "./include/*.h ./include/*.hpp")

source_group("Header Files" FILES ${Lagrangian3D_HEADER_FILES})

add_library(lagrangian3d STATIC "${Lagrangian3D_SOURCE_FILES}" "${Lagrangian3D_HEADER_FILES}")
target_include_directories(lagrangian3d PRIVATE "./include")

# OpenMP
if(OpenMP_CXX_FOUND)
    target_link_libraries(lagrangian3d OpenMP::OpenMP_CXX)
endif()

# common
target_link_libraries(lagrangian3d common)

# glfw
target_link_libraries(lagrangian3d "${PROJECT_SOURCE_DIR}/third_party/glfw/lib/glfw3.lib")