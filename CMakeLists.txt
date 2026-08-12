cmake_minimum_required(VERSION 3.20)
project(VulkanDemo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_executable(VulkanDemo main.cpp)
target_include_directories(VulkanDemo PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    "C:/cpp-libraries/glfw-3.5.1.bin.WIN64/include"
)
target_link_directories(VulkanDemo PRIVATE
    "C:/cpp-libraries/glfw-3.5.1.bin.WIN64/lib-vc2026"
)
target_link_libraries(VulkanDemo PRIVATE ${Vulkan_LIBRARIES} glfw3)