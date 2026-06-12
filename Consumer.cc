#include <iostream>
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <string>
#include "OssManager.h"

using namespace std;
using namespace AmqpClient;

void get_msg(){
    string uri = "amqp://guest:guest@localhost:5672/%2f";
    Channel::ptr_t channel = Channel::CreateFromUri(uri);
    // 方式2：推送模式--- 等待RabbitMQ推送消息 (阻塞式)
    const string& q = "oss.queue";

    channel->BasicConsume(q);// 订阅队列
                             // 阻塞：等待RabbitMQ推送消息
    while(1){
        Envelope::ptr_t envelope = channel->BasicConsumeMessage();
        // 打印消息
        if (envelope && envelope->Message()) {
            string oss_name=envelope->Message()->Body();
            string file_name=oss_name;
            cout<<file_name<<endl;
            OssManager::getInstance().upload(oss_name,file_name);
        }
    }
}

int main()
{
    // 加载配置文件
    if (!Config::getInstance().load("config.json")) {
        cerr << "Error: Failed to load config.json!" << endl;
        return -1;
    }
    get_msg();
}

