# OpenSSL 入门指南

OpenSSL 是一个开源的加密库，用于数据加密、哈希计算、TLS/SSL 通信，它是 HTTPS、VPN、SSH 等安全协议的核心组件。OpenSSL 提供了 C 语言编程的 API，我们可以使用这些 API 进行哈希计算、数据加密和 TLS 连接等操作 (TLS 连接是 HTTPS 的基础，我们不讲…)。

在使用 OpenSSL C API 之前，我们需要事先安装 OpenSSL 以及它的开发库：

```shell
sudo apt install openssl
$ sudo apt install libssl-dev  # 安装 OpenSSL 开发库
```

链接时，需要加上 `-lssl` 和 `-lcrypto`：

```shell
gcc main.c -o main -lssl -lcrypto
```

## 1. 哈希计算

OpenSSL 3.0 之前，我们可以使用 `XXX_Init()`, `XXX_Update()`, `XXX_Final()` 等接口来计算哈希值 (在 OpenSSL 3.0 中，这些接口已经被废弃)：

```c
void sha256_hash(const char* data)
{
    SHA256_CTX context;			// 创建 SHA256 上下文
    SHA256_Init(&context);		// 初始化上下文
    SHA256_Update(&context, data, strlen(data));	// 更新上下文
 
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &context);					// 计算哈希值
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}
```

OpenSSL 3.0 以及更新的版本推荐使用 EVP (Envelop) 接口编程：

```c
#include <openssl/evp.h>

void sha256_hash(const char *data) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();		// 创建 EVP 上下文
    unsigned char hash[EVP_MAX_MD_SIZE];	// EVP_MAX_MD_SIZE: 最大哈希长度
    unsigned int hash_len;					// 用来接收实际的哈希长度

    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);		// 初始化上下文，采用 sha256 哈希算法
    EVP_DigestUpdate(ctx, data, strlen(data));		// 更新上下文
    EVP_DigestFinal_ex(ctx, hash, &hash_len);		// 计算哈希值

    printf("SHA-256: ");
    for (int i = 0; i < hash_len; i++) {			// 转换成十六进制字符
        printf("%02x", hash[i]);
    }
    printf("\n");

    EVP_MD_CTX_free(ctx);							// 释放上下文
}
```

## 2. 数据加密

我们通过一个示例给大家讲解一下，如何使用 OpenSSL 进行 **加密/解密**。`AES-256-CBC` 是一种对称加密算法，加密和解密都使用 256 位密钥 (32 字节) 和初始化向量 `IV`(16 字节)。

```c
#include <func.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define AES_KEY_SIZE 32     // 256-bit 密钥
#define AES_IV_SIZE  16     // 128-bit IV
#define BUFFER_SIZE  1024   // 缓冲区大小

int aes256_encrypt(const unsigned char* plaintext, int plaintext_len, 
                   const unsigned char* key, const unsigned char* iv,
                   unsigned char* ciphertext)
{
    EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();     // 创建上下
    // 初始化 AES-256-CBC 加密
    EVP_EncryptInit_ex(context, EVP_aes_256_cbc(), NULL, key, iv); 
    
    // 加密
    int len, ciphertext_len;
    EVP_EncryptUpdate(context, ciphertext, &len, plaintext, plaintext_len);
    ciphertext_len = len;
    // 处理填充部分
    EVP_EncryptFinal_ex(context, ciphertext + len, &len);   
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(context);
    return ciphertext_len;
}

int aes256_decrypt(const unsigned char* ciphertext, int ciphertext_len, 
                   const unsigned char* key, const unsigned char* iv,
                   unsigned char* plaintext)
{
    EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new(); // 创建上下文
    
    int len, plaintext_len;
    // 初始化 AES_256_CBC 解密
    EVP_DecryptInit_ex(context, EVP_aes_256_cbc(), NULL, key, iv);
    // 解密
    EVP_DecryptUpdate(context, plaintext, &len, ciphertext, ciphertext_len);
    plaintext_len = len;

    // 处理填充部分
    EVP_DecryptFinal_ex(context, plaintext + len, &len);
    plaintext_len += len;

    EVP_CIPHER_CTX_free(context);
    return plaintext_len;
}

int main(int argc, char* argv[])
{
    unsigned char key[AES_KEY_SIZE];    // 256-bit 密钥
    unsigned char iv[AES_IV_SIZE];      // 128-bit IV
    // 生成随机密钥和 IV
    RAND_bytes(key, sizeof(key));
    RAND_bytes(iv, sizeof(iv));

    const char* plaintext = "I love xixi";
    unsigned char ciphertext[BUFFER_SIZE];
    unsigned char decryptedtext[BUFFER_SIZE];
    
    // 加密
    printf("plaintext: %s\n", plaintext);
    int ctext_len = aes256_encrypt((unsigned char*)plaintext, strlen(plaintext), key, iv, ciphertext);
    printf("ciphertext (Hex): ");
    for (int i = 0; i < ctext_len; i++) {
        printf("%02x", ciphertext[i]);
    }
    printf("\n");
    
    // 解密
    int dtext_len = aes256_decrypt(ciphertext, ctext_len, key, iv, decryptedtext);
    decryptedtext[dtext_len] = '\0';    // 添加空字符
    printf("decryptedtext: %s\n", decryptedtext);
    return 0;
}
```

注意事项：1. 加密后的数据不再是字符串，而是二进制数据

## 3. Token 认证

Token 认证（Token Authentication）是一种基于令牌（Token）进行身份验证的机制。它是一种无状态的身份验证方式，即服务器不会存储用户的登录状态，而是依赖客户端携带的 Token 来识别用户身份。

在 Token 认证机制下，用户首次登录时，服务器会颁发一个 Token，并要求用户在后续的请求中携带该 Token 进行身份认证。服务器通过解析 Token 来确认用户身份并执行相应的操作。

### 3.1 Token 认证流程

Token 认证的典型流程如下：

1. **用户登录**
   - 用户提供用户名和密码，向服务器发送登录请求。
2. **服务器验证身份**
   - 服务器检查用户提供的用户名和密码是否正确。
   - 如果验证成功，服务器生成一个 Token 并返回给用户。
3. **用户携带 Token 访问受保护资源**
   - 之后，用户每次请求受保护资源时，都会在参数中携带 Token。
4. **服务器验证 Token**
   - 服务器检查 Token 是否有效 (通常包括签名验证、是否过期等)。
   - 如果 Token 合法，则允许访问资源；否则，返回 (未授权) 错误。
5. **Token 过期**
   - 如果 Token 过期，用户需要重新登录获取新的 Token，或者使用刷新 Token 机制延长会话。

### 3.2 常见的 Token 类型

目前常见的 Token 机制包括：

#### (1) JWT

JWT 的全称叫 JSON Web Token，它的结构如下：

```json
Header.Payload.Signature
```

- **Header**：定义 Token 类型和加密算法，如：

  ```json
  {
    "alg": "HS256",
    "typ": "JWT"
  }
  ```
  
- **Payload**：存储用户信息和声明 (claims)，内容由用户自定义，如：

  ```json
  {
    "sub": "1234567890",	// 主题
    "name": "John Doe",	// 名字
    "iat": 1516239022,	// 签发时间
    "exp": 1616239022		// 过期时间
  }
  ```
  
- **Signature**：使用 Header 中指定的算法（如 HMAC SHA256）对 `Header + Payload` 进行加密，生成签名。

- **特点**：

  - 采用 **无状态认证**，服务器不存储会话数据。
  - **自包含**，Token 本身存储了用户信息。
  - **支持签名**，防止篡改。
  - **可设置过期时间**，增强安全性。

#### (2) OAuth 2.0 访问令牌

OAuth 2.0 是一种授权协议，广泛用于 API 访问控制，访问令牌用于代表用户访问受保护的资源。

- **访问令牌 (Acesss Token)**：用于身份认证，通常有较短的过期时间。
- **刷新令牌 (Refresh Token)**：用于获取新的 Access Token，避免频繁登录。

### 3.3 用 C 语言实现 Token 认证

当然，我们可以用 OpenSSL 提供的 C API 来实现 Token 认证机制，不过，这将是一个很繁琐的过程。我们可以使用 `libjwt` 库来简化创建和验证 JWT 的流程。

首先，安装 `libjwt` 开发库：

```shell
sudo apt install libjwt-dev
```

```c
#include <jwt.h>

void generate_jwt_token(const char* secret_key)
{
    jwt_t* jwt;
    jwt_new(&jwt);  // 创建 JWT

    // 设置算法为 HS256
    jwt_set_alg(jwt, JWT_ALG_HS256, (unsigned char*)secret_key, strlen(secret_key));

    // 设置载荷(Payload): 用户自定义数据
    jwt_add_grant(jwt, "sub", "subject"); 
    jwt_add_grant(jwt, "username", "peanutixx");	// 用户ID
    jwt_add_grant(jwt, "role", "admin");    		// 用户角色
    jwt_add_grant_int(jwt, "exp", time(NULL) + 3600);   // 过期时间 (1小时)
    
    char* token = jwt_encode_str(jwt);		// token长度是不确定的，100-300字节
    printf("Generated JWT: %s\n", token);

    // 释放资源
    jwt_free(jwt);
    free(token);
}

void verify_jwt(const char* token, const char* secret_key)
{
    jwt_t* jwt;
    int err = jwt_decode(&jwt, token, (unsigned char*)secret_key, strlen(secret_key));
    if (err) {
        printf("Invalid JWT!\n");
        return ;
    }

    // 获取 Payload
    printf("Subject: %s\n", jwt_get_grant(jwt, "sub"));
    printf("Username: %s\n", jwt_get_grant(jwt, "username"));
    printf("Role: %s\n", jwt_get_grant(jwt, "role"));
    printf("Expiration Time: %ld\n", jwt_get_grant_int(jwt, "exp"));

    jwt_free(jwt);
}
```



## 4. 注册/登录流程

### 4.1 注册

(1) 客户端：输入用户名/密码/确认密码，加密 (项目中可选)，发送给服务器

(2) 服务器：解密 (项目中可选)，获取用户名和密码，检查用户名是否存在
	存在：返回用户名已存在
	不存在：生成一个随机的盐值 (避免撞库攻击)，根据盐值和密码计算哈希值，在 `users` 表中插入一条记录 (存储密码的哈希值)

### 4.2 登录

(1) 客户端：输入用户名和密码，加密 (项目中可选)，发送给服务器

(2) 服务器：解密 (项目中可选)，获取用户名和密码，查表获取用户的盐值，根据盐值和密码计算哈希值，检查哈希值是否一致：
	一致：登录成功，根据 `secret_key` 和用户信息生成 `Token`，并把 `Token` 返回给客户端。
	不一致：登录失败

(3) 登录成功之后，客户端每次访问服务器，都会携带 `Token` 信息。服务器通过校验 `Token` 来获取用户身份以及辨别用户权限…

