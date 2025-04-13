#ifndef SPEECH_RECOGNIZER_H
#define SPEECH_RECOGNIZER_H

#include <string>
#include <functional>
#include <thread>
#include <atomic>

class SpeechRecognizer {
public:
    explicit SpeechRecognizer(const std::string& model_path = "model", float sample_rate = 16000.0f);
    ~SpeechRecognizer();

    void setCallback(std::function<void(const std::string&)> cb);
    void start();  
    void stop();   

private:
    void recognizeLoop();  // Thread function for recognition loop

    std::string modelPath;
    float sampleRate;
    std::function<void(const std::string&)> callback;

    std::thread worker;
    std::atomic<bool> running;
};

#endif // SPEECH_RECOGNIZER_H
