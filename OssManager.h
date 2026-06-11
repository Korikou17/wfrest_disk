#pragma once
#include <alibabacloud/oss/OssClient.h>

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
    OssManager() {}
    ~OssManager(){}
    static OssManager *m_pInstance;
};

