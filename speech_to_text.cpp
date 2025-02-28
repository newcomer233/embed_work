
/*
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <regex>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <jwt-cpp/jwt.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

using json = nlohmann::json;

// HTTP响应回调函数，用于curl写入数据
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t total = size * nmemb;
    s->append(static_cast<char*>(contents), total);
    return total;
}

// 读取二进制文件（用于读取录音文件）
std::vector<unsigned char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件: " + filename);
    }
    return std::vector<unsigned char>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

// OpenSSL Base64编码函数
std::string base64_encode(const std::vector<unsigned char>& data) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // 禁用换行符
    BIO_write(bio, data.data(), data.size());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    return result;
}

// 直接从文件读取私钥（debug_private_key.pem）
std::string readPrivateKeyFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开私钥文件: " + filename);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// 获取访问令牌函数（JWT生成 + OAuth请求）
std::string getAccessToken() {
    // 从环境变量获取服务账号JSON文件路径
    const char* key_path = std::getenv("GOOGLE_APPLICATION_CREDENTIALS");
    if (!key_path) {
        std::cerr << "❌ 错误：未设置环境变量 GOOGLE_APPLICATION_CREDENTIALS" << std::endl;
        return "";
    }
    std::cout << "✅ 环境变量路径: " << key_path << std::endl;

    // 读取服务账号JSON文件
    std::ifstream key_file(key_path);
    if (!key_file.is_open()) {
        std::cerr << "❌ 错误：无法打开密钥文件" << std::endl;
        return "";
    }
    std::cout << "✅ 成功打开密钥文件" << std::endl;
    json service_account;
    try {
        service_account = json::parse(key_file);
    } catch (const std::exception& e) {
        std::cerr << "❌ JSON解析错误: " << e.what() << std::endl;
        return "";
    }
    std::cout << "✅ 成功解析JSON密钥文件" << std::endl;

    // 直接从文件读取私钥（确保该文件已生成并格式正确）
    std::string private_key;
    try {
        private_key = readPrivateKeyFromFile("debug_private_key.pem");
    } catch (const std::exception& e) {
        std::cerr << "❌ 读取私钥文件错误: " << e.what() << std::endl;
        return "";
    }
    // 修剪首尾空白
    private_key = std::regex_replace(private_key, std::regex("^\\s+|\\s+$"), "");
    std::cout << "\n直接读取私钥前50字符: " << private_key.substr(0, 50) << "..." << std::endl;

    // 输出当前系统时间
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::cout << "🕒 系统时间: " << std::ctime(&now_time);

    // 检查系统时间偏差（允许10分钟内）
    if (std::chrono::system_clock::now() < now - std::chrono::minutes(10) ||
        std::chrono::system_clock::now() > now + std::chrono::minutes(10)) {
        std::cerr << "❌ 严重错误：系统时间偏差超过10分钟！" << std::endl;
        return "";
    }

    std::string token;
    try {
        // 使用jwt-cpp生成JWT令牌，注意这里只传入私钥字符串，其它参数留空
        token = jwt::create()
            .set_issuer(service_account["client_email"])
            .set_audience("https://oauth2.googleapis.com/token")
            .set_payload_claim("scope", jwt::claim(std::string("https://www.googleapis.com/auth/cloud-platform")))
            .set_issued_at(now)
            .set_expires_at(now + std::chrono::hours(1))
            .sign(jwt::algorithm::rs256(private_key, "", "", ""));
        std::cout << "✅ 成功生成JWT令牌（前50字符）: " << token.substr(0, 50) << "..." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ JWT错误详情: " << e.what() << std::endl;
        return "";
    }

    // 使用生成的JWT令牌请求Google OAuth访问令牌
    CURL* curl = curl_easy_init();
    std::string response;
    std::string data = "grant_type=urn:ietf:params:oauth:grant-type:jwt-bearer&assertion=" + token;
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "❌ CURL错误: " << curl_easy_strerror(res) << std::endl;
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    std::cout << "🔄 OAuth响应原始数据: " << response << std::endl;
    json result;
    try {
        result = json::parse(response);
    } catch (const std::exception& e) {
        std::cerr << "❌ JSON解析错误: " << e.what() << std::endl;
        return "";
    }
    
    if (result.contains("access_token")) {
        return result["access_token"];
    } else {
        std::cerr << "❌ 认证错误: " << response << std::endl;
        return "";
    }
}

int main() {
    // 1. 录制音频（5秒）——确保系统已安装 sox 工具（rec 命令）
    std::cout << "\n🎤 开始录音（5秒）..." << std::endl;
    system("rec -r 16000 -c 1 -b 16 audio.flac trim 0 5");

    try {
        // 2. 读取音频文件
        auto audioData = readFile("audio.flac");
        std::cout << "\n✅ 成功读取音频文件（" << audioData.size() << "字节）" << std::endl;

        // 3. Base64编码
        std::string base64Audio = base64_encode(audioData);
        std::cout << "🔢 Base64编码完成（前50字符）: " << base64Audio.substr(0, 50) << "..." << std::endl;

        // 4. 构建语音识别请求的JSON数据
        json request = {
            {"audio", {{"content", base64Audio}}},
            {"config", {
                {"encoding", "FLAC"},
                {"sampleRateHertz", 16000},
                {"languageCode", "zh-CN"}
            }}
        };

        // 5. 获取访问令牌
        std::cout << "\n🔑 正在获取访问令牌..." << std::endl;
        std::string token = getAccessToken();
        if (token.empty()) return 1;
        std::cout << "\n✅ 访问令牌获取成功（前50字符）: " << token.substr(0, 50) << "..." << std::endl;

        // 6. 发送语音识别请求
        std::cout << "\n📡 正在发送识别请求..." << std::endl;
        CURL* curl = curl_easy_init();
        std::string response;
        if (curl) {
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
            curl_easy_setopt(curl, CURLOPT_URL, "https://speech.googleapis.com/v1/speech:recognize");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            std::string request_str = request.dump();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_str.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "❌ CURL错误: " << curl_easy_strerror(res) << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        // 7. 解析并输出语音识别结果
        std::cout << "\n🔍 正在解析结果..." << std::endl;
        json result;
        try {
            result = json::parse(response);
            if (result.contains("results") && !result["results"].empty()) {
                std::cout << "\n🎉 识别结果: " 
                          << result["results"][0]["alternatives"][0]["transcript"] 
                          << std::endl;
            } else {
                std::cerr << "❌ API错误: " << result.dump(2) << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "❌ JSON解析错误: " << e.what() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ 主程序错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
*/


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <jwt-cpp/jwt.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

using json = nlohmann::json;

// HTTP响应回调函数
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t total = size * nmemb;
    s->append(static_cast<char*>(contents), total);
    return total;
}

// 读取二进制文件
std::vector<unsigned char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件: " + filename);
    }
    return std::vector<unsigned char>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

// Base64 编码
std::string base64_encode(const std::vector<unsigned char>& data) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data.data(), data.size());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    return result;
}

// 获取访问令牌（JWT + OAuth）
std::string getAccessToken() {
    // 获取 Google 服务账户 JSON 文件路径
    const char* key_path = std::getenv("GOOGLE_APPLICATION_CREDENTIALS");
    if (!key_path) {
        std::cerr << "❌ 错误：未设置 GOOGLE_APPLICATION_CREDENTIALS" << std::endl;
        return "";
    }
    
    std::ifstream key_file(key_path);
    if (!key_file.is_open()) {
        std::cerr << "❌ 错误：无法打开密钥文件" << std::endl;
        return "";
    }

    json service_account;
    try {
        service_account = json::parse(key_file);
    } catch (const std::exception& e) {
        std::cerr << "❌ JSON解析错误: " << e.what() << std::endl;
        return "";
    }

    // 直接从 JSON 读取私钥
    std::string private_key = service_account["private_key"].get<std::string>();

    // 获取当前时间戳
    auto now = std::chrono::system_clock::now();

    // 生成 JWT 令牌
    std::string jwt_token;
    try {
        jwt_token = jwt::create()
            .set_issuer(service_account["client_email"].get<std::string>())
            .set_audience("https://oauth2.googleapis.com/token")
            .set_payload_claim("scope", jwt::claim(std::string("https://www.googleapis.com/auth/cloud-platform")))
            .set_issued_at(now)
            .set_expires_at(now + std::chrono::hours(1))
            .sign(jwt::algorithm::rs256("", private_key, "", ""));
    } catch (const std::exception& e) {
        std::cerr << "❌ JWT 生成错误: " << e.what() << std::endl;
        return "";
    }

    // 发送 OAuth 令牌请求
    CURL* curl = curl_easy_init();
    std::string response;
    std::string data = "grant_type=urn:ietf:params:oauth:grant-type:jwt-bearer&assertion=" + jwt_token;
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "❌ CURL错误: " << curl_easy_strerror(res) << std::endl;
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    json result;
    try {
        result = json::parse(response);
        if (result.contains("access_token")) {
            return result["access_token"];
        } else {
            std::cerr << "❌ 认证错误: " << response << std::endl;
            return "";
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ JSON解析错误: " << e.what() << std::endl;
        return "";
    }
}

int main() {
    // 1. 录制音频（使用 rec 录制 5 秒 FLAC）
    std::cout << "\n🎤 录音中（5秒）..." << std::endl;
    system("rec -r 16000 -c 1 -b 16 audio.flac trim 0 5");

    try {
        // 2. 读取音频文件
        auto audioData = readFile("audio.flac");

        // 3. Base64 编码
        std::string base64Audio = base64_encode(audioData);

        // 4. 构建语音识别请求
        json request = {
            {"audio", {{"content", base64Audio}}},
            {"config", {
                {"encoding", "FLAC"},
                {"sampleRateHertz", 16000},
                {"languageCode", "zh-CN"}
            }}
        };

        // 5. 获取访问令牌
        std::string token = getAccessToken();
        if (token.empty()) return 1;

        // 6. 发送语音识别请求
        CURL* curl = curl_easy_init();
        std::string response;
        if (curl) {
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
            curl_easy_setopt(curl, CURLOPT_URL, "https://speech.googleapis.com/v1/speech:recognize");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            std::string request_str = request.dump();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_str.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "❌ CURL错误: " << curl_easy_strerror(res) << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        // 7. 解析识别结果
        json result = json::parse(response);
        if (result.contains("results") && !result["results"].empty()) {
            std::cout << "\n🎉 识别结果: " 
                      << result["results"][0]["alternatives"][0]["transcript"] 
                      << std::endl;
        } else {
            std::cerr << "❌ API错误: " << result.dump(2) << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ 主程序错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

