#include <string>
#include "OssManager.h"
#include <memory>

using namespace std;

OssManager &OssManager::getInstance()
    {
        static OssManager instance;
        return instance;
    }


bool OssManager::upload(string oss_name,string file_name)
{
    string bucketName_ = "korikou";
    m_client.SetRegion("cn-wuhan");
    auto outcome = m_client.PutObject(bucketName_,oss_name,file_name);
    if (!outcome.isSuccess()) {
        cout << "PutObject FAILED"
            << ", code:" << outcome.error().Code()
            << ", message:" << outcome.error().Message()
            << ", requestId:" << outcome.error().RequestId() << endl;
        return false;
    }
    return true;
}

