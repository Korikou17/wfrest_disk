#include <string>
#include "OssManager.h"
#include <memory>

using namespace std;

OssManager &OssManager::getInstance()
    {
        static OssManager instance;
        return instance;
    }


bool OssManager::upload(string filename,string content)
{
    string bucketName_ = "korikou";
    m_client.SetRegion("cn-wuhan");
    shared_ptr<iostream> stream = make_shared<stringstream>(move(content));
    PutObjectRequest request(bucketName_, filename, stream);
    auto outcome = m_client.PutObject(request);
    if (!outcome.isSuccess()) {
        cout << "PutObject FAILED"
            << ", code:" << outcome.error().Code()
            << ", message:" << outcome.error().Message()
            << ", requestId:" << outcome.error().RequestId() << endl;
        return false;
    }
    return true;
}

