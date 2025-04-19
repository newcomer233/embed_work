#ifndef SPEECH_RECOGNIZER_H
#define SPEECH_RECOGNIZER_H

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include "vosk_api.h"
#include <mutex>

class SpeechRecognizer {
public:
    explicit SpeechRecognizer(const std::string& model_path = "model", float sample_rate = 16000.0f);
    ~SpeechRecognizer();

    void setCallback(std::function<void(const std::string&)> cb);
    void setPartialCallback(std::function<void(const std::string&)> cb);
    void start();  // start
    void stop();   // stop

    void setInitialGrammar(const std::vector<std::string>& commands);
    void setGrammar(const std::vector<std::string>& commands);
private:
    void recognizeLoop();  // main threads

    VoskModel* model = nullptr;
    VoskRecognizer* recognizer = nullptr;

    std::string modelPath;
    float sampleRate;
    
    std::function<void(const std::string&)> callback;
    std::function<void(const std::string&)> partialCallback;

    std::thread worker;
    std::atomic<bool> running;
    std::vector<std::string> initialGrammar;
    std::mutex recognizerMutex;
};

#endif // SPEECH_RECOGNIZER_H
