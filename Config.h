#pragma once
#include <string>

// 单例模式，全局配置读取
class Config
{
public:
    static Config& getInstance();

    // 从 JSON 文件加载配置
    bool load(const std::string& configPath);

    // ========== OSS 配置 ==========
    std::string ossEndpoint()      const;
    std::string ossAccessKeyId()   const;
    std::string ossAccessKeySecret() const;
    std::string ossRegion()        const;
    std::string ossBucketName()    const;

    // ========== 数据库配置 ==========
    std::string databaseURL()      const;

    // ========== 服务器配置 ==========
    unsigned short serverPort()    const;

private:
    Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    // OSS
    std::string ossEndpoint_;
    std::string ossAccessKeyId_;
    std::string ossAccessKeySecret_;
    std::string ossRegion_;
    std::string ossBucketName_;

    // Database
    std::string databaseURL_;

    // Server
    unsigned short serverPort_ = 8888;
};
