#include "SpeechRecognizer.h"
#include "vosk_api.h"
#include <iostream>
#include <cstdio>
#include <cstring>

SpeechRecognizer::SpeechRecognizer(const std::string& model_path, float sample_rate)
    : modelPath(model_path), sampleRate(sample_rate), running(false) {}

SpeechRecognizer::~SpeechRecognizer() {
    stop();
}

void SpeechRecognizer::setCallback(std::function<void(const std::string&)> cb) {
    callback = cb;
}

void SpeechRecognizer::start() {
    if (running) return;
    running = true;
    worker = std::thread(&SpeechRecognizer::recognizeLoop, this);
}

void SpeechRecognizer::stop() {
    if (!running) return;
    running = false;
    if (worker.joinable())
        worker.join();
}

void SpeechRecognizer::recognizeLoop() {
    VoskModel* model = vosk_model_new(modelPath.c_str());
    if (!model) {
        std::cerr << "❌ Failed to load model from " << modelPath << std::endl;
        return;
    }

    VoskRecognizer* recognizer = vosk_recognizer_new(model, sampleRate);
    if (!recognizer) {
        std::cerr << "❌ Failed to create recognizer" << std::endl;
        vosk_model_free(model);
        return;
    }

    FILE* pipe = popen("arecord -q -f S16_LE -r 16000 -c 1", "r");
    if (!pipe) {
        std::cerr << "❌ Failed to open arecord pipe" << std::endl;
        vosk_recognizer_free(recognizer);
        vosk_model_free(model);
        return;
    }

    char buffer[4096];
    while (running && !feof(pipe)) {
        size_t nread = fread(buffer, 1, sizeof(buffer), pipe);
        if (nread > 0) {
            if (vosk_recognizer_accept_waveform(recognizer, buffer, nread)) {
                const char* result = vosk_recognizer_result(recognizer);
                if (callback) callback(result);
            } else {
                // const char* partial = vosk_recognizer_partial_result(recognizer);
                // 可选：处理 partial 识别
            }
        }
    }

    std::string finalResult = vosk_recognizer_final_result(recognizer);
    if (callback) callback(finalResult);

    pclose(pipe);
    vosk_recognizer_free(recognizer);
    vosk_model_free(model);
}
