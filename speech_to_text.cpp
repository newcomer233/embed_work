/*

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <jwt-cpp/jwt.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

using json = nlohmann::json;

// HTTP响应回调
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t total = size * nmemb;
    s->append(static_cast<char*>(contents), total);
    return total;
}

std::vector<unsigned char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) throw std::runtime_error("无法打开文件: " + filename);
    
    const std::streamsize size = file.tellg();
    if (size <= 0) throw std::runtime_error("无效文件大小: " + filename);
    
    std::vector<unsigned char> buffer(size);
    file.seekg(0, std::ios::beg);
    
    // 正确的读取和状态检查
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("读取失败: " + filename);
    }
    
    return buffer;
}
// Base64编码
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

// 获取访问令牌
std::string getAccessToken() {
    const char* key_path = std::getenv("GOOGLE_APPLICATION_CREDENTIALS");
    if (!key_path) {
        std::cerr << "❌ Error: GOOGLE_APPLICATION_CREDENTIALS not set" << std::endl;
        return "";
    }
    
    std::ifstream key_file(key_path);
    if (!key_file.is_open()) {
        std::cerr << "❌ Error: Cannot open service account key file" << std::endl;
        return "";
    }

    json service_account;
    try {
        service_account = json::parse(key_file);
    } catch (const std::exception& e) {
        std::cerr << "❌ JSON parse error: " << e.what() << std::endl;
        return "";
    }

    std::string private_key = service_account["private_key"].get<std::string>();
    auto now = std::chrono::system_clock::now();

    try {
        std::string jwt_token = jwt::create()
            .set_issuer(service_account["client_email"].get<std::string>())
            .set_audience("https://oauth2.googleapis.com/token")
            .set_payload_claim("scope", jwt::claim(std::string("https://www.googleapis.com/auth/cloud-platform")))
            .set_issued_at(now)
            .set_expires_at(now + std::chrono::hours(1))
            .sign(jwt::algorithm::rs256("", private_key, "", ""));

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
                std::cerr << "❌ CURL error: " << curl_easy_strerror(res) << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        json result = json::parse(response);
        if (result.contains("access_token")) {
            return result["access_token"];
        } else {
            std::cerr << "❌ Authentication error: " << response << std::endl;
            return "";
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ JWT error: " << e.what() << std::endl;
        return "";
    }
}

// 关键词检测
void checkKeywords(const std::string& transcript) {
    const std::vector<std::string> KEYWORDS = {
        "open", "close", "stop", "start", "help", 
        "activate", "shutdown", "cancel", "enable", "disable"
    };

    std::string processed = transcript;
    std::transform(processed.begin(), processed.end(), processed.begin(),
        [](unsigned char c){ return std::tolower(c); });
    processed.erase(std::remove_if(processed.begin(), processed.end(), ::ispunct), processed.end());

    std::istringstream iss(processed);
    std::string word;
    while (iss >> word) {
        if (std::find(KEYWORDS.begin(), KEYWORDS.end(), word) != KEYWORDS.end()) {
            std::cout << "\n🔑 Detected keyword: " << word << std::endl;
            // 在此添加自定义操作
        }
    }
}

int main() {
    std::cout << "\n🎤 Waiting for speech input (say something to start)..." << std::endl;
    int recResult = system(
        "rec -r 16000 -c 1 -b 16 audio.flac silence 1 0.2 3% 1 1.5 3% 2>/dev/null"
    );

    if (recResult != 0) {
        std::cerr << "❌ Recording failed or no speech detected" << std::endl;
        return 1;
    }

    try {
        auto audioData = readFile("audio.flac");
        std::string base64Audio = base64_encode(audioData);

        json request = {
            {"audio", {{"content", base64Audio}}},
            {"config", {
                {"encoding", "FLAC"},
                {"sampleRateHertz", 16000},
                {"languageCode", "en-US"},
                {"enableAutomaticPunctuation", true},
                {"model", "command_and_search"}
            }}
        };

        std::string token = getAccessToken();
        if (token.empty()) return 1;

        CURL* curl = curl_easy_init();
        std::string response;
        if (curl) {
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
            curl_easy_setopt(curl, CURLOPT_URL, "https://speech.googleapis.com/v1/speech:recognize");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.dump().c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "❌ CURL error: " << curl_easy_strerror(res) << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        json result = json::parse(response);
        if (result.contains("results") && !result["results"].empty()) {
            std::string transcript = result["results"][0]["alternatives"][0]["transcript"];
            std::cout << "\n🎉 Recognition result: " << transcript << std::endl;
            checkKeywords(transcript);
        } else {
            std::cerr << "❌ API error: " << result.dump(2) << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ Main error: " << e.what() << std::endl;
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
#include <algorithm>
#include <cctype>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <jwt-cpp/jwt.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

using json = nlohmann::json;

// 增强错误处理的HTTP响应回调
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t total = size * nmemb;
    if (total > 0) {
        s->append(static_cast<char*>(contents), total);
    }
    return total;
}

// 安全的文件读取实现
std::vector<unsigned char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("无法打开文件: " + filename);
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (size <= 0) {
        throw std::runtime_error("文件为空: " + filename);
    }
    
    std::vector<unsigned char> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("读取文件失败: " + filename);
    }
    return buffer;
}

// 增强的Base64编码实现
std::string base64_encode(const std::vector<unsigned char>& data) {
    if (data.empty()) {
        throw std::invalid_argument("输入数据为空");
    }
    
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data.data(), data.size());
    (void)BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    
    return result;
}

// 改进的JWT令牌生成实现
std::string getAccessToken() {
    const char* key_path = std::getenv("GOOGLE_APPLICATION_CREDENTIALS");
    if (!key_path) {
        throw std::runtime_error("环境变量GOOGLE_APPLICATION_CREDENTIALS未设置");
    }
    
    std::ifstream key_file(key_path);
    if (!key_file) {
        throw std::runtime_error("无法打开服务账号密钥文件");
    }
    
    json service_account;
    try {
        service_account = json::parse(key_file);
    } catch (const json::exception& e) {
        throw std::runtime_error("JSON解析失败: " + std::string(e.what()));
    }
    
    std::string private_key = service_account.value("private_key", "");
    if (private_key.empty()) {
        throw std::runtime_error("私钥字段不存在");
    }
    
    // 修复转义换行符问题
    size_t pos = 0;
    while ((pos = private_key.find("\\n", pos)) != std::string::npos) {
        private_key.replace(pos, 2, "\n");
        pos += 1;
    }
    
    auto now = std::chrono::system_clock::now();
    
    try {
        auto token = jwt::create()
            .set_issuer(service_account["client_email"].get<std::string>())
            .set_audience("https://oauth2.googleapis.com/token")
            .set_payload_claim("scope", jwt::claim(std::string("https://www.googleapis.com/auth/cloud-platform")))
            .set_issued_at(now)
            .set_expires_at(now + std::chrono::hours(1))
            .sign(jwt::algorithm::rs256("", private_key, "", ""));
        
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("CURL初始化失败");
        }
        
        std::string response;
        std::string data = "grant_type=urn:ietf:params:oauth:grant-type:jwt-bearer&assertion=" + token;
        
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        
        curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // 10秒超时
        
        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            throw std::runtime_error("CURL请求失败: " + std::string(curl_easy_strerror(res)));
        }
        
        json result = json::parse(response);
        if (!result.contains("access_token")) {
            throw std::runtime_error("认证失败: " + response);
        }
        return result["access_token"].get<std::string>();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("JWT处理失败: " + std::string(e.what()));
    }
}

// 改进的关键词检测逻辑
void checkKeywords(const std::string& transcript) {
    const std::vector<std::string> KEYWORDS = {
        "open", "close", "stop", "start", "help",
        "activate", "shutdown", "cancel", "enable", "disable"
    };
    
    std::string processed;
    processed.reserve(transcript.size());
    
    std::transform(transcript.begin(), transcript.end(), std::back_inserter(processed),
        [](unsigned char c) { return std::tolower(c); });
    
    processed.erase(std::remove_if(processed.begin(), processed.end(),
        [](unsigned char c) { return std::ispunct(c); }), processed.end());
    
    std::istringstream iss(processed);
    std::string word;
    while (iss >> word) {
        if (std::find(KEYWORDS.begin(), KEYWORDS.end(), word) != KEYWORDS.end()) {
            std::cout << "\n🔑 检测到关键词: " << word << std::endl;
            // 在此添加自定义操作
        }
    }
}

// 主程序入口
int main() {
    try {
        std::cout << "\n🎤 等待语音输入（说话开始录音）..." << std::endl;
        
        // 改进的录音命令处理
        const std::string rec_command = 
            "rec -r 16000 -c 1 -b 16 audio.flac silence 1 0.2 3% 1 1.5 3% 2>rec_error.log";
        int rec_status = system(rec_command.c_str());
        
        if (rec_status != 0) {
            std::ifstream error_file("rec_error.log");
            std::string error_msg((std::istreambuf_iterator<char>(error_file)),
                std::istreambuf_iterator<char>());
            throw std::runtime_error("录音失败:\n" + error_msg);
        }
        
        // 安全加载音频数据
        auto audioData = readFile("audio.flac");
        std::cout << "✅ 音频文件大小: " << audioData.size() << " 字节" << std::endl;
        
        std::string base64Audio;
        try {
            base64Audio = base64_encode(audioData);
        } catch (const std::exception& e) {
            throw std::runtime_error("Base64编码失败: " + std::string(e.what()));
        }
        
        // 构造API请求
        json request = {
            {"audio", {{"content", base64Audio}}},
            {"config", {
                {"encoding", "FLAC"},
                {"sampleRateHertz", 16000},
                {"languageCode", "en-US"},
                {"enableAutomaticPunctuation", true},
                {"model", "command_and_search"}
            }}
        };
        
        // 获取访问令牌
        std::string token;
        try {
            token = getAccessToken();
        } catch (const std::exception& e) {
            throw std::runtime_error("令牌获取失败: " + std::string(e.what()));
        }
        std::cout << "✅ 访问令牌获取成功" << std::endl;
        
        // 发送API请求
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("CURL初始化失败");
        }
        
        std::string response;
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
        
        curl_easy_setopt(curl, CURLOPT_URL, "https://speech.googleapis.com/v1/speech:recognize");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.dump().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L); // 15秒超时
        
        CURLcode res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            throw std::runtime_error("API请求失败: " + std::string(curl_easy_strerror(res)));
        }
        if (http_code != 200) {
            throw std::runtime_error("API返回错误: HTTP " + std::to_string(http_code));
        }
        
        // 安全解析API响应
        json result;
        try {
            result = json::parse(response);
        } catch (const json::exception& e) {
            throw std::runtime_error("JSON解析失败: " + std::string(e.what()));
        }
        
        if (!result.contains("results")) {
            throw std::runtime_error("API响应缺少results字段");
        }
        if (!result["results"].is_array() || result["results"].empty()) {
            std::cout << "\n⚠️ 未检测到有效语音" << std::endl;
            return 0;
        }
        
        auto& firstResult = result["results"][0];
        if (!firstResult.contains("alternatives")) {
            throw std::runtime_error("结果缺少alternatives字段");
        }
        if (!firstResult["alternatives"].is_array() || firstResult["alternatives"].empty()) {
            std::cout << "\n⚠️ 无识别结果" << std::endl;
            return 0;
        }
        
        std::string transcript = firstResult["alternatives"][0].value("transcript", "");
        if (transcript.empty()) {
            std::cout << "\n⚠️ 识别内容为空" << std::endl;
            return 0;
        }
        
        std::cout << "\n🎉 识别结果: " << transcript << std::endl;
        checkKeywords(transcript);
        
    } catch (const std::exception& e) {
        std::cerr << "\n❌ 严重错误: " << e.what() << std::endl;
        return 1;
    }
    
    // 清理临时文件
    std::remove("audio.flac");
    std::remove("rec_error.log");
    return 0;
}



