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

void OssManager::init(const string& accessKeyId,const string& accessKeySecret)
{
    accessKeyId_     = accessKeyId;
    accessKeySecret_ = accessKeySecret;
}

bool OssManager::upload(string filename,string content)
{
    string endpoint_ ="oss-cn-wuhan-lr.aliyuncs.com";
    string region_ = "cn-wuhan";
    string bucketName_ = "korikou";
    ClientConfiguration conf;
    OssClient client(endpoint_, accessKeyId_, accessKeySecret_, conf);
    client.SetRegion(region_);

    shared_ptr<iostream> stream = make_shared<stringstream>(move(content));
    PutObjectRequest request(bucketName_, filename, stream);
    auto outcome = client.PutObject(request);
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
