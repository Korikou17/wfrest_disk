#!/bin/bash
# =============================================
# CloudDisk 一键启动脚本
# 启动顺序: auth_service → oss_server → api_gateway
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
LOG_DIR="$PROJECT_DIR/logs"

mkdir -p "$PID_DIR" "$LOG_DIR"

# ------------------ 端口配置 ------------------
AUTH_PORT=1412
API_PORT=8888

# ------------------ 前置检查 ------------------
echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  CloudDisk 服务启动前检查${NC}"
echo -e "${CYAN}========================================${NC}"

# 检查二进制文件
check_binary() {
    local bin="$1"
    if [ ! -f "$PROJECT_DIR/$bin" ]; then
        echo -e "${RED}[ERROR] 找不到 $bin，请先运行 ./build.sh${NC}"
        exit 1
    fi
}
check_binary "auth_service"
check_binary "oss_server"
check_binary "api_gateway"

# 检查 MySQL
echo -ne "检查 MySQL ... "
if mysqladmin ping -h127.0.0.1 --silent 2>/dev/null; then
    echo -e "${GREEN}运行中${NC}"
else
    echo -e "${YELLOW}未检测到${NC}"
    echo -e "${YELLOW}[WARN] 请确保 MySQL 已启动 (端口 3306)${NC}"
fi

# 检查 RabbitMQ
echo -ne "检查 RabbitMQ ... "
if curl -s -o /dev/null -w "%{http_code}" http://localhost:15672 2>/dev/null | grep -q "200"; then
    echo -e "${GREEN}运行中${NC}"
else
    echo -e "${YELLOW}未检测到${NC}"
    echo -e "${YELLOW}[WARN] 请确保 RabbitMQ 已启动 (端口 5672)${NC}"
fi

# 检查端口是否被占用
check_port() {
    local port="$1"
    if ss -tlnp 2>/dev/null | grep -q ":$port "; then
        return 0
    fi
    return 1
}

# 检查 PID 文件中的进程是否存活
is_running() {
    local pid_file="$1"
    if [ -f "$pid_file" ]; then
        local pid
        pid=$(cat "$pid_file")
        if kill -0 "$pid" 2>/dev/null; then
            return 0
        fi
    fi
    return 1
}

# ------------------ 启动函数 ------------------
start_service() {
    local name="$1"
    local bin="$2"
    local pid_file="$PID_DIR/${name}.pid"
    local log_file="$LOG_DIR/${name}.log"
    local port="$3"

    # 检查端口是否已被占用
    if [ -n "$port" ] && check_port "$port"; then
        echo -e "${YELLOW}[SKIP] ${name} 端口 ${port} 已被占用，跳过启动${NC}"
        return 0
    fi

    # 检查是否已在运行
    if is_running "$pid_file"; then
        echo -e "${YELLOW}[SKIP] ${name} 已在运行中 (PID: $(cat "$pid_file"))${NC}"
        return 0
    fi

    echo -ne "启动 ${name} ... "

    # 切换到项目目录启动（确保相对路径资源正确加载）
    cd "$PROJECT_DIR"
    nohup "$PROJECT_DIR/$bin" >> "$log_file" 2>&1 &
    local pid=$!
    echo "$pid" > "$pid_file"

    # 等待一小会儿检查是否启动成功
    sleep 1
    if kill -0 "$pid" 2>/dev/null; then
        echo -e "${GREEN}OK${NC} (PID: $pid)"
        return 0
    else
        echo -e "${RED}FAILED${NC}"
        echo -e "${RED}      查看日志: tail -f ${log_file}${NC}"
        rm -f "$pid_file"
        return 1
    fi
}

# ------------------ 启动服务 ------------------
echo ""
echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  启动 CloudDisk 服务${NC}"
echo -e "${CYAN}========================================${NC}"

STARTED_COUNT=0
FAILED_SERVICES=""

# 1. 启动 auth_service (SRPC 端口 1412) —— 关键服务，必须成功
if start_service "auth_service" "auth_service" "$AUTH_PORT"; then
    STARTED_COUNT=$((STARTED_COUNT + 1))
else
    FAILED_SERVICES="$FAILED_SERVICES auth_service"
    echo -e "${RED}[FATAL] auth_service 启动失败，后续服务无法启动${NC}"
    exit 1
fi

# 2. 启动 oss_server (RabbitMQ 消费者) —— 非关键，配置缺失时跳过
OSS_CONFIG="$PROJECT_DIR/OssService/config.json"
OSS_EXAMPLE="$PROJECT_DIR/OssService/config.json.example"
if [ ! -f "$OSS_CONFIG" ]; then
    echo -e "${YELLOW}[WARN] 跳过 oss_server: 缺少配置文件${NC}"
    echo -e "${YELLOW}       请复制 ${OSS_EXAMPLE} 为 config.json 并填入真实凭证${NC}"
else
    if start_service "oss_server" "oss_server" ""; then
        STARTED_COUNT=$((STARTED_COUNT + 1))
    else
        echo -e "${YELLOW}[WARN] oss_server 启动失败 (非关键服务，继续启动网关)${NC}"
        echo -e "${YELLOW}       查看日志: tail -f ${LOG_DIR}/oss_server.log${NC}"
    fi
fi

# 3. 启动 api_gateway (HTTP 端口 8888) —— 关键服务，必须成功
if start_service "api_gateway" "api_gateway" "$API_PORT"; then
    STARTED_COUNT=$((STARTED_COUNT + 1))
else
    FAILED_SERVICES="$FAILED_SERVICES api_gateway"
    echo -e "${RED}[ERROR] api_gateway 启动失败${NC}"
    echo -e "${RED}       查看日志: tail -f ${LOG_DIR}/api_gateway.log${NC}"
fi

# ------------------ 启动完成 ------------------
echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  CloudDisk 启动完成 (${STARTED_COUNT} 个服务)${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo -e "  ${CYAN}API 网关:   ${NC}http://localhost:${API_PORT}"
echo -e "  ${CYAN}认证服务:   ${NC}localhost:${AUTH_PORT} (SRPC)"
echo -e "  ${CYAN}OSS 服务:   ${NC}RabbitMQ 消费者${NC}"

if [ -n "$FAILED_SERVICES" ]; then
    echo ""
    echo -e "  ${RED}未成功启动:${NC}$FAILED_SERVICES"
fi

echo ""
echo -e "  ${CYAN}日志目录:   ${NC}${LOG_DIR}"
echo -e "  ${CYAN}PID 目录:   ${NC}${PID_DIR}"
echo ""
echo -e "  ${YELLOW}查看日志:${NC}"
echo -e "    tail -f ${LOG_DIR}/api_gateway.log"
echo -e "    tail -f ${LOG_DIR}/auth_service.log"
echo -e "    tail -f ${LOG_DIR}/oss_server.log"
echo ""
echo -e "  ${YELLOW}停止服务:${NC}  ./stop.sh"
