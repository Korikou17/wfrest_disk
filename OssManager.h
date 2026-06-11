#pragma once
#include <alibabacloud/oss/OssClient.h>
#include <string>
#include <memory>
#include "Config.h"

using namespace std;
using namespace AlibabaCloud::OSS;

class OssManager
{
public:
    static OssManager *getInstance();

    static void destroyInstance();

    OssManager(const OssManager&)=delete;
    OssManager& operator=(const OssManager &)=delete;
    
    bool upload(string filename,string content);
private:
    OssManager()
    :conf()
    ,m_client("oss-cn-wuhan-lr.aliyuncs.com", Config::getInstance().ossAccessKeyId(),Config::getInstance().ossAccessKeySecret(),conf)
    {}
    ~OssManager(){}
    static OssManager *m_pInstance;
    ClientConfiguration conf;
    OssClient m_client;
};

