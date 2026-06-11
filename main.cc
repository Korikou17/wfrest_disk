#include "CloudDiskServer.h"
#include "Config.h"
#include "OssManager.h"
#include <iostream>
#include <signal.h>

using namespace std;

WFFacilities::WaitGroup waitGroup(1);

void sig_handler(int)
{
    waitGroup.done();
}

int main()
{
    signal(SIGINT, sig_handler);
    srand(time(NULL)); // 设置随机种子

    // 加载配置文件
    if (!Config::getInstance().load("config.json")) {
        cerr << "Error: Failed to load config.json!" << endl;
        return -1;
    }

// 使用配置初始化 OSS
    OssManager::getInstance()->init(
        Config::getInstance().ossAccessKeyId(),
        Config::getInstance().ossAccessKeySecret()
    );

    CloudDiskServer server;

    // 注册路由
    server.register_routes();

    if (server.start(8888) == 0) {
        server.list_routes();
        waitGroup.wait();
        server.stop();
    } else {
        cerr << "Error: Server start FAILED!" << endl;
    }
}
