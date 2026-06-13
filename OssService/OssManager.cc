#include "OssManager.h"
#include <pthread.h>
#include "Config.h"
using namespace std;

// 初始化静态成员变量
OssManager* OssManager::instance = nullptr;
pthread_mutex_t OssManager::mutex = PTHREAD_MUTEX_INITIALIZER;

OssManager* OssManager::get_instance()
{
    // 第一次检查：如果实例已经存在，直接返回
    if (instance == nullptr) {
        pthread_mutex_lock(&mutex);
        // 第二次检查：防止在多线程环境下重复创建实例
        if (instance == nullptr) {
            instance = new OssManager();
        }
        pthread_mutex_unlock(&mutex);
    }
    return instance;
}

void OssManager::destroy_instance()
{
    pthread_mutex_lock(&mutex);
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
        AlibabaCloud::OSS::ShutdownSdk(); // 释放SDK资源
    }
    pthread_mutex_unlock(&mutex);
}

OssManager::OssManager()
{
    // 1. 初始化SDK
    AlibabaCloud::OSS::InitializeSdk();

    string endpoint = "oss-cn-wuhan-lr.aliyuncs.com";
    string region = "cn-wuhan";
    string accessKeyId = Config::getInstance().ossAccessKeyId();
    string accessKeySecret = Config::getInstance().ossAccessKeySecret();
    AlibabaCloud::OSS::ClientConfiguration conf;
    client_ = std::make_unique<AlibabaCloud::OSS::OssClient>(endpoint, accessKeyId, accessKeySecret, conf);

    client_->SetRegion(region);
}

bool OssManager::upload_object(const string& bucket, const string& object, const string& file) // 上传文件
{
    auto outcome = client_->PutObject(bucket, object, file);
    return outcome.isSuccess();
}

bool OssManager::upload_object(const string& bucket, const string& object, std::shared_ptr<std::iostream> content)
{
    auto outcome = client_->PutObject(bucket, object, content);
    return outcome.isSuccess();
}

