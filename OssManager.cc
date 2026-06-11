#include <string>
#include "OssManager.h"
#include <memory>

using namespace std;

OssManager *OssManager::getInstance()
    {
        if(!m_pInstance)
        {
            m_pInstance=new OssManager{};
            InitializeSdk();
            atexit(&destroyInstance);
        }
        return m_pInstance;
    }

void OssManager::destroyInstance()
    {
        if(m_pInstance)
        {
            ShutdownSdk();
            cout<<"AliyunSDK Shutdown"<<endl;
            delete m_pInstance;
            m_pInstance=nullptr;
        }
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

OssManager *OssManager::m_pInstance=nullptr;
