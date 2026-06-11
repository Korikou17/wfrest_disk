#include "Config.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

Config& Config::getInstance()
{
    static Config instance;
    return instance;
}

bool Config::load(const string& configPath)
{
    ifstream ifs(configPath);
    if (!ifs.is_open()) {
        cerr << "[Config] Cannot open config file: " << configPath << endl;
        return false;
    }

    json j;
    try {
        ifs >> j;
    } catch (const json::parse_error& e) {
        cerr << "[Config] JSON parse error: " << e.what() << endl;
        return false;
    }

    // ========== 读取 OSS 配置 ==========
    if (j.contains("oss")) {
        auto& oss = j["oss"];
        if (oss.contains("endpoint"))          ossEndpoint_          = oss["endpoint"].get<string>();
        if (oss.contains("accessKeyId"))       ossAccessKeyId_       = oss["accessKeyId"].get<string>();
        if (oss.contains("accessKeySecret"))   ossAccessKeySecret_   = oss["accessKeySecret"].get<string>();
        if (oss.contains("region"))            ossRegion_            = oss["region"].get<string>();
        if (oss.contains("bucketName"))        ossBucketName_        = oss["bucketName"].get<string>();
    }

    // ========== 读取数据库配置 ==========
    if (j.contains("database")) {
        auto& db = j["database"];
        if (db.contains("url"))               databaseURL_          = db["url"].get<string>();
    }

    // ========== 读取服务器配置 ==========
    if (j.contains("server")) {
        auto& srv = j["server"];
        if (srv.contains("port"))             serverPort_           = srv["port"].get<unsigned short>();
    }

    cout << "[Config] Configuration loaded successfully from: " << configPath << endl;
    return true;
}

// ========== OSS 配置 getter ==========
string Config::ossEndpoint()        const { return ossEndpoint_; }
string Config::ossAccessKeyId()     const { return ossAccessKeyId_; }
string Config::ossAccessKeySecret() const { return ossAccessKeySecret_; }
string Config::ossRegion()          const { return ossRegion_; }
string Config::ossBucketName()      const { return ossBucketName_; }

// ========== 数据库配置 getter ==========
string Config::databaseURL()        const { return databaseURL_; }

// ========== 服务器配置 getter ==========
unsigned short Config::serverPort() const { return serverPort_; }
