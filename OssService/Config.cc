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
        if (oss.contains("accessKeyId"))       ossAccessKeyId_       = oss["accessKeyId"].get<string>();
        if (oss.contains("accessKeySecret"))   ossAccessKeySecret_   = oss["accessKeySecret"].get<string>();
    }

    cout << "[Config] Configuration loaded successfully from: " << configPath << endl;
    return true;
}

// ========== OSS 配置 getter ==========
string Config::ossAccessKeyId()     const { return ossAccessKeyId_; }
string Config::ossAccessKeySecret() const { return ossAccessKeySecret_; }

