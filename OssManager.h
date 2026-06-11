#pragma once
#include <alibabacloud/oss/OssClient.h>
#include <string>

using namespace std;

using namespace AlibabaCloud::OSS;

class OssManager
{
public:
    static OssManager *getInstance();

    static void destroyInstance();

    OssManager(const OssManager&)=delete;
    OssManager& operator=(const OssManager &)=delete;

    // 从配置初始化 OSS 参数
    void init(const string& endpoint,
              const string& accessKeyId,
              const string& accessKeySecret,
              const string& region,
              const string& bucketName);
    
    bool upload(string filename,string content);

private:
    OssManager() {}
    ~OssManager(){}
    static OssManager *m_pInstance;

    string endpoint_;
    string accessKeyId_;
    string accessKeySecret_;
    string region_;
    string bucketName_;
};

