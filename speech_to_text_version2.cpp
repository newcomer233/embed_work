#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <jwt-cpp/jwt.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <unordered_set>
#include "webrtc_vad.h"
#include <unistd.h>
#include <cmath>


using namespace std;
using json = nlohmann::json;

// 常量
const string AUDIO_FILENAME = "audio.flac";
const string TRANSCRIPT_FILENAME = "transcript.txt";
const string API_URL = "https://speech.googleapis.com/v1/speech:recognize";
const string OAUTH_URL = "https://oauth2.googleapis.com/token";
const int RECORD_DURATION = 10; // 录音时长

// 关键词列表
const vector<string> keywords = {"hello", "turn on", "turn off", "start", "stop"};

//函数声明
static size_t WriteCallback(void*, size_t, size_t, string*);
vector<unsigned char> readFile(const string&);
string base64_encode(const vector<unsigned char>&);
string getAccessToken();
string sendRequestToAPI(const json&, const string&);
void saveTranscriptToFile(const string&);
bool isHumanVoice(const vector<unsigned char>&);
bool containsKeyword(const string&, const vector<string>&);
double computeRMS(const vector<int16_t>& audio);


//计算音频 RMS
double computeRMS(const vector<int16_t>& audio) {
    if (audio.empty()) return 0.0;

    double sum = 0.0;
    for (auto sample : audio) {
        sum += sample * sample;
    }
    return sqrt(sum / audio.size());
}

//人声检测
bool isHumanVoice(const vector<unsigned char>& audioData) {
    if (audioData.empty()) {
        cerr << "❌ 错误: 传入的音频数据为空" << endl;
        return false;
    }

    VadInst* vad = WebRtcVad_Create();
    if (!vad) {
        cerr << "❌ VAD 创建失败！" << endl;
        return false;
    }

    if (WebRtcVad_Init(vad) != 0) {
        cerr << "❌ VAD 初始化失败！" << endl;
        WebRtcVad_Free(vad);
        return false;
    }

    WebRtcVad_set_mode(vad, 0);

    // 转换为16-bit PCM
    vector<int16_t> pcmAudio(audioData.size() / sizeof(int16_t));
    memcpy(pcmAudio.data(), audioData.data(), audioData.size());

    // **计算 RMS 过滤静音数据**
    double rms = computeRMS(pcmAudio);
    cout << "🔍 音频 RMS 值: " << rms << endl;

    if (rms < 800) { // 📌 **如果 RMS 低于 800，跳过检测**
        cout << "⚠️ 低音量，可能是静音数据，跳过检测" << endl;
        WebRtcVad_Free(vad);
        return false;
    }

    bool hasVoice = false;
    for (size_t i = 0; i + 160 <= pcmAudio.size(); i += 160) {
        if (WebRtcVad_Process(vad, 16000, &pcmAudio[i], 160) == 1) {
            hasVoice = true;
            break;
        }
    }

    WebRtcVad_Free(vad);
    return hasVoice;
}

//录音 
void recordAudio() {
    system(("rec -r 16000 -c 1 -b 16 " + AUDIO_FILENAME + " trim 0 " + to_string(RECORD_DURATION)).c_str());
}


bool containsKeyword(const string& transcript, const vector<string>& keywords) {
    for (const auto& keyword : keywords) {
        if (transcript.find(keyword) != string::npos) {
            return true;
        }
    }
    return false;
}


static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* s) {
    size_t total = size * nmemb;
    if (total > 0) {
        s->append(static_cast<char*>(contents), total);
    }
    return total;
}

vector<unsigned char> readFile(const string& filename) {
    ifstream file(filename, ios::binary | ios::ate);
    if (!file) {
        throw runtime_error("无法打开文件: " + filename);
    }
    
    streamsize size = file.tellg();
    file.seekg(0, ios::beg);
    
    vector<unsigned char> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

string base64_encode(const vector<unsigned char>& data) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data.data(), data.size());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    
    return result;
}

string getAccessToken() {
    const char* key_path = getenv("GOOGLE_APPLICATION_CREDENTIALS");
    if (!key_path) {
        throw runtime_error("环境变量 GOOGLE_APPLICATION_CREDENTIALS 未设置");
    }

    ifstream key_file(key_path);
    json service_account = json::parse(key_file);
    string private_key = service_account["private_key"].get<string>();

    size_t pos = 0;
    while ((pos = private_key.find("\\n", pos)) != string::npos) {
        private_key.replace(pos, 2, "\n");
        pos++;
    }

    auto now = chrono::system_clock::now();
    auto token = jwt::create()
        .set_issuer(service_account["client_email"].get<string>())
        .set_audience(OAUTH_URL)
        .set_payload_claim("scope", jwt::claim(string("https://www.googleapis.com/auth/cloud-platform")))
        .set_issued_at(now)
        .set_expires_at(now + chrono::hours(1))
        .sign(jwt::algorithm::rs256("", private_key, "", ""));

    CURL* curl = curl_easy_init();
    string response;
    string data = "grant_type=urn:ietf:params:oauth:grant-type:jwt-bearer&assertion=" + token;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    
    curl_easy_setopt(curl, CURLOPT_URL, OAUTH_URL.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    json result = json::parse(response);
    return result.value("access_token", "");
}

string sendRequestToAPI(const json& requestJson, const string& token) {
    CURL* curl = curl_easy_init();
    if (!curl) throw runtime_error("CURL 初始化失败");

    string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    string requestBody = requestJson.dump();
    curl_easy_setopt(curl, CURLOPT_URL, API_URL.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        throw runtime_error("CURL 发送请求失败: " + string(curl_easy_strerror(res)));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}


void saveTranscriptToFile(const string& transcript) {
    ofstream file(TRANSCRIPT_FILENAME);
    if (!file) {
        throw runtime_error("无法创建或写入文件: " + TRANSCRIPT_FILENAME);
    }
    file << transcript;
    file.close();
}


// 📌 **主函数**
int main() {
    try {
        cout << "🎤 正在监听人声..." << endl;
        while (true) {
            system("rec -r 16000 -c 1 -b 16 temp_audio.wav trim 0 1");

            ifstream file("temp_audio.wav", ios::binary | ios::ate);
            if (!file) {
                cerr << "❌ 录音失败，无法打开 temp_audio.wav" << endl;
                continue;
            }

            streamsize size = file.tellg();
            file.seekg(0, ios::beg);
            vector<unsigned char> buffer(size);
            file.read(reinterpret_cast<char*>(buffer.data()), size);

            if (isHumanVoice(buffer)) {
                cout << "✅ 检测到人声，开始录音..." << endl;
                recordAudio();
                break;
            } else {
                cout << "⏳ 未检测到人声，继续监听..." << endl;
            }
            usleep(100000);
        }

        auto audioData = readFile(AUDIO_FILENAME);
        string base64Audio = base64_encode(audioData);
        string token = getAccessToken();

        json request = {{"config", {{"encoding", "FLAC"}, {"sampleRateHertz", 16000}, {"languageCode", "en-US"}, {"enableAutomaticPunctuation", true}, {"model", "command_and_search"}}}, {"audio", {{"content", base64Audio}}}};

        string response = sendRequestToAPI(request, token);
        json responseJson = json::parse(response);

        string transcript = responseJson["results"][0]["alternatives"][0].value("transcript", "");
        cout << "🎉 识别结果: " << transcript << endl;

    } catch (const exception& e) {
        cerr << "❌ 错误: " << e.what() << endl;
        return 1;
    }
    return 0;
}

