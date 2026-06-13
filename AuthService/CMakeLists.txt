# 设置CMake的最小版本
cmake_minimum_required(VERSION 3.15)

# 项目名称
#   项目可以包含多个目标(target)
#   target: 可执行程序, 静态库, 动态库
project(CloudDisk)

# 指定C++的版本: -std=c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置可执行程序生成的路径 (项目根目录)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/..)

# 添加头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR})

# 生成的可执行程序: 认证微服务
add_executable(auth_service
    auth_service.pb.cc
    CryptoUtil.cc
    Auth_server.cc
)

# 设置编译选项
target_compile_options(auth_service PRIVATE
    -fno-rtti
    -g
)

# 链接库
target_link_libraries(auth_service PRIVATE
    curl
    crypto
    pthread
    ssl
    jwt
    wfrest
    srpc
    lz4
    snappy
    protobuf
    workflow
)
