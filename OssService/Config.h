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
    std::string ossAccessKeyId()   const;
    std::string ossAccessKeySecret() const;

private:
    Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    // OSS
    std::string ossAccessKeyId_;
    std::string ossAccessKeySecret_;
};
