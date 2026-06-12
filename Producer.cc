#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include "Producer.h"

using namespace AmqpClient;

void Producer::send_msg(string file_path){
        string host = "127.0.0.1";
        int port = 5672;
        string username = "guest";
        string password = "guest";
        string vhost = "/";
        Channel::ptr_t channel = Channel::Create(host, port, username, password, vhost);
        BasicMessage::ptr_t message = BasicMessage::Create(file_path);
        string exchange = "oss.direct"; // 交换机
        string routingKey = "oss"; // 消息的routingKey
        channel->BasicPublish(exchange, routingKey, message); // 发布消息
}

