#include "OssManager.h"
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <iostream>
#include <string>
#include "Config.h"

using namespace std;
using namespace AmqpClient;

int main()
{
    // 加载配置文件
    if (!Config::getInstance().load("config.json")) {
        cerr << "Error: Failed to load config.json!" << endl;
        return -1;
    }

    string uri = "amqp://guest:guest@localhost:5672/%2f";
    const string& q = "oss.queue";

    Channel::ptr_t channel = Channel::CreateFromUri(uri);

    // 订阅队列，队列中有消息，RabbitMQ会主动推送给消费者
    channel->BasicConsume(q);

    OssManager* oss = OssManager::get_instance();
    for (;;) {
        // 等待RabbitMQ推送消息
        Envelope::ptr_t envelope = channel->BasicConsumeMessage();

        if (envelope && envelope->Message()) {
            string filepath = envelope->Message()->Body(); // 文件路径
            string object = filepath; // 对象名
            // 上传文件
            if (!oss->upload_object("korikou", object, filepath)) {
                cerr << "Error: upload " << filepath << " FAILED!" << endl;
            }
        }
    }

    OssManager::destroy_instance();
}

