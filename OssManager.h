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
    void init(const string& accessKeyId,const string& accessKeySecret);
    
    bool upload(string filename,string content);

private:
    OssManager() {}
    ~OssManager(){}
    static OssManager *m_pInstance;

    string accessKeyId_;
    string accessKeySecret_;
};

