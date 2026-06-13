#!/bin/bash
# =============================================
# CloudDisk 一键停止脚本
# 停止顺序: api_gateway → oss_server → auth_service
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
PID_DIR="$PROJECT_DIR/.pids"

# ------------------ 停止函数 ------------------
stop_service() {
    local name="$1"
    local pid_file="$PID_DIR/${name}.pid"
    local force="${2:-0}"

    if [ ! -f "$pid_file" ]; then
        echo -e "${YELLOW}[SKIP] ${name} 未找到 PID 文件，可能未运行${NC}"
        return 0
    fi

    local pid
    pid=$(cat "$pid_file")

    if ! kill -0 "$pid" 2>/dev/null; then
        echo -e "${YELLOW}[SKIP] ${name} (PID: $pid) 已停止${NC}"
        rm -f "$pid_file"
        return 0
    fi

    echo -ne "停止 ${name} (PID: $pid) ... "

    if [ "$force" -eq 1 ]; then
        kill -9 "$pid" 2>/dev/null
    else
        # 先发 SIGINT (Ctrl+C) 优雅退出
        kill -INT "$pid" 2>/dev/null
    fi

    # 等待进程退出（最多 10 秒）
    local waited=0
    while kill -0 "$pid" 2>/dev/null && [ $waited -lt 10 ]; do
        sleep 0.5
        waited=$((waited + 1))
    done

    # 如果还没退出，强制 kill
    if kill -0 "$pid" 2>/dev/null; then
        echo -ne "超时，强制终止... "
        kill -9 "$pid" 2>/dev/null
        sleep 1
    fi

    if kill -0 "$pid" 2>/dev/null; then
        echo -e "${RED}FAILED${NC}"
        return 1
    else
        echo -e "${GREEN}OK${NC}"
        rm -f "$pid_file"
        return 0
    fi
}

# ------------------ 端口强制清理 ------------------
kill_by_port() {
    local port="$1"
    local pids
    pids=$(ss -tlnp 2>/dev/null | grep ":$port " | sed -n 's/.*pid=\([0-9]*\).*/\1/p' | sort -u)
    if [ -n "$pids" ]; then
        for pid in $pids; do
            echo -e "  清理端口 ${port} 上的进程 (PID: $pid)"
            kill -9 "$pid" 2>/dev/null || true
        done
    fi
}

# ------------------ 执行停止 ------------------
echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  停止 CloudDisk 服务${NC}"
echo -e "${CYAN}========================================${NC}"

# 反向停止：先停网关，再停消费者，最后停认证服务
stop_service "api_gateway"
stop_service "oss_server"
stop_service "auth_service"

# ------------------ 可选：强制清理僵尸进程和端口 ------------------
FORCE=0
if [ "$1" = "-f" ] || [ "$1" = "--force" ]; then
    FORCE=1
fi

if [ $FORCE -eq 1 ]; then
    echo ""
    echo -e "${YELLOW}强制清理端口占用...${NC}"
    kill_by_port 8888
    kill_by_port 1412
fi

# 清理残留的 PID 文件
if [ $FORCE -eq 1 ]; then
    rm -f "$PID_DIR"/*.pid
fi

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  CloudDisk 全部服务已停止${NC}"
echo -e "${GREEN}========================================${NC}"

# 显示残留进程（如果有）
REMAINING=$(ps aux 2>/dev/null | grep -E '(api_gateway|auth_service|oss_server)' | grep -v grep | grep -v stop.sh || true)
if [ -n "$REMAINING" ]; then
    echo ""
    echo -e "${YELLOW}[WARN] 仍有残留进程:${NC}"
    echo "$REMAINING"
    echo ""
    echo -e "${YELLOW}使用 ./stop.sh -f 强制清理${NC}"
fi
