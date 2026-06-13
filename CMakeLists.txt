# ===================================================
# CloudDisk 统一构建文件
# 包含三个 target:
#   1. api_gateway  — HTTP API 网关 (端口 8888)
#   2. auth_service — 认证微服务   (SRPC 端口 1412)
#   3. oss_server   — OSS 上传服务 (RabbitMQ 消费者)
# ===================================================
cmake_minimum_required(VERSION 3.15)
project(CloudDisk)

# ------------------ C++ 标准 ------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------ 输出目录：所有二进制生成到项目根目录 ------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

# ------------------ 全局头文件搜索路径 ------------------
include_directories(${PROJECT_SOURCE_DIR})             # common.h, CloudDiskServer.h
include_directories(${PROJECT_SOURCE_DIR}/AuthService) # CryptoUtil.h, auth_service.*.h
include_directories(${PROJECT_SOURCE_DIR}/OssService)  # Config.h, OssManager.h

# ==================== Target 1: api_gateway ====================
add_executable(api_gateway
    main.cc
    CloudDiskServer.cc
    AuthService/CryptoUtil.cc
    AuthService/auth_service.pb.cc
)

target_compile_options(api_gateway PRIVATE
    -fno-rtti
    -g
)

target_link_libraries(api_gateway PRIVATE
    alibabacloud-oss-cpp-sdk
    curl
    crypto
    pthread
    ssl
    jwt
    wfrest
    SimpleAmqpClient
    srpc
    lz4
    snappy
    protobuf
    workflow
)

# ==================== Target 2: auth_service ====================
add_executable(auth_service
    AuthService/Auth_server.cc
    AuthService/CryptoUtil.cc
    AuthService/auth_service.pb.cc
)

target_compile_options(auth_service PRIVATE
    -fno-rtti
    -g
)

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

# ==================== Target 3: oss_server ====================
add_executable(oss_server
    OssService/Ossuploader.cc
    OssService/Config.cc
    OssService/OssManager.cc
)

target_compile_options(oss_server PRIVATE
    -fno-rtti
    -g
)

target_link_libraries(oss_server PRIVATE
    alibabacloud-oss-cpp-sdk
    curl
    crypto
    pthread
    SimpleAmqpClient
)
