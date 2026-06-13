#!/bin/bash
# =============================================
# CloudDisk 构建脚本 (统一 CMake 项目)
# 用法:
#   ./build.sh              # 默认构建所有目标 (Debug)
#   ./build.sh release      # Release 构建
#   ./build.sh clean        # 清理构建产物
#   ./build.sh api_gateway  # 仅构建 api_gateway
#   ./build.sh auth_service # 仅构建 auth_service
#   ./build.sh oss_server   # 仅构建 oss_server
# =============================================

set -e

# ------------------ 颜色 ------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

# ------------------ 路径 ------------------
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

# ------------------ 默认参数 ------------------
CLEAN=0
BUILD_TYPE="Debug"
TARGET=""

# ------------------ 解析命令行参数 ------------------
while [[ $# -gt 0 ]]; do
    case "$1" in
        clean)
            CLEAN=1
            shift
            ;;
        release|Release)
            BUILD_TYPE="Release"
            shift
            ;;
        debug|Debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        api_gateway|auth_service|oss_server)
            TARGET="$1"
            shift
            ;;
        -h|--help)
            echo "用法: $0 [clean] [release|debug] [api_gateway|auth_service|oss_server]"
            echo ""
            echo "选项:"
            echo "  clean          清理 build 目录和编译产物"
            echo "  release        Release 模式构建 (-O2)"
            echo "  debug          Debug 模式构建 (默认, -g)"
            echo "  api_gateway    仅构建 API 网关"
            echo "  auth_service   仅构建认证微服务"
            echo "  oss_server     仅构建 OSS 上传服务"
            echo ""
            echo "示例:"
            echo "  $0                          # 构建所有 (Debug)"
            echo "  $0 release                  # 构建所有 (Release)"
            echo "  $0 clean release            # 清理后 Release 构建"
            echo "  $0 api_gateway              # 仅构建 api_gateway"
            exit 0
            ;;
        *)
            echo -e "${RED}[ERROR] 未知参数: $1${NC}"
            echo "使用 -h 或 --help 查看帮助"
            exit 1
            ;;
    esac
done

# ------------------ 清理 ------------------
if [ $CLEAN -eq 1 ]; then
    echo -e "${YELLOW}[CLEAN] 正在清理编译产物...${NC}"
    rm -rf "$BUILD_DIR"
    rm -f "$PROJECT_DIR/api_gateway"
    rm -f "$PROJECT_DIR/auth_service"
    rm -f "$PROJECT_DIR/oss_server"
    echo -e "${GREEN}[CLEAN] 清理完成${NC}"
    if [ -z "$TARGET" ] && [ "$BUILD_TYPE" = "Debug" ]; then
        exit 0
    fi
fi

# ------------------ CMake 配置 ------------------
echo ""
echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  CloudDisk 统一构建${NC}"
echo -e "${CYAN}  源码目录: ${PROJECT_DIR}${NC}"
echo -e "${CYAN}  构建类型: ${BUILD_TYPE}${NC}"
if [ -n "$TARGET" ]; then
    echo -e "${CYAN}  构建目标: ${TARGET}${NC}"
else
    echo -e "${CYAN}  构建目标: 全部 (api_gateway + auth_service + oss_server)${NC}"
fi
echo -e "${CYAN}========================================${NC}"
echo ""

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 首次运行或 CMakeLists.txt 变更后需要重新配置
echo -e "${YELLOW}[CMake] 正在配置项目 ...${NC}"
if ! cmake "$PROJECT_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"; then
    echo -e "${RED}[ERROR] CMake 配置失败${NC}"
    cd "$PROJECT_DIR"
    exit 1
fi

# ------------------ 编译 ------------------
NPROC=$(nproc 2>/dev/null || echo 4)
echo ""
echo -e "${YELLOW}[Make] 正在编译 (并行任务数: ${NPROC}) ...${NC}"

if [ -n "$TARGET" ]; then
    # 仅编译指定 target
    if ! make -j"$NPROC" "$TARGET"; then
        echo -e "${RED}[ERROR] 编译失败: ${TARGET}${NC}"
        cd "$PROJECT_DIR"
        exit 1
    fi
else
    # 编译所有
    if ! make -j"$NPROC"; then
        echo -e "${RED}[ERROR] 编译失败${NC}"
        cd "$PROJECT_DIR"
        exit 1
    fi
fi

cd "$PROJECT_DIR"

# ------------------ 构建摘要 ------------------
echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  构建完成${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

echo -e "${YELLOW}生成的二进制文件:${NC}"
for bin in api_gateway auth_service oss_server; do
    if [ -f "$PROJECT_DIR/$bin" ]; then
        SIZE=$(du -h "$PROJECT_DIR/$bin" | cut -f1)
        echo -e "  ${GREEN}✓${NC} ${bin}  (${SIZE})"
    else
        if [ -z "$TARGET" ] || [ "$TARGET" = "$bin" ]; then
            echo -e "  ${RED}✗${NC} ${bin}  (未生成)"
        else
            echo -e "  ${CYAN}—${NC} ${bin}  (本次未构建)"
        fi
    fi
done

echo ""
echo -e "${CYAN}提示:${NC}"
echo -e "  ./build.sh clean   — 清理编译产物"
echo -e "  ./start.sh         — 一键启动服务"
