#include "SpeechRecognizer.h"
#include "vosk_api.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <sstream>

SpeechRecognizer::SpeechRecognizer(const std::string& model_path, float sample_rate)
    : modelPath(model_path), sampleRate(sample_rate), running(false) 
{
    model = vosk_model_new(modelPath.c_str());
    if (!model) {
        std::cerr << "Failed to load model from " << modelPath << std::endl;
    }
}

SpeechRecognizer::~SpeechRecognizer() {
    stop();
    if (recognizer) {
        vosk_recognizer_free(recognizer);
        recognizer = nullptr;
    }
    if (model) {
        vosk_model_free(model);
        model = nullptr;
    }
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

    if (!model) {
        std::cerr << "Failed to load model from " << modelPath << std::endl;
        return;
    }

    recognizer = vosk_recognizer_new(model, sampleRate);

    if (!recognizer) {
        std::cerr << "Failed to create recognizer" << std::endl;
        return;
    }

    FILE* pipe = popen("arecord -q -f S16_LE -r 16000 -c 1", "r");
    if (!pipe) {
        std::cerr << "Failed to open arecord pipe" << std::endl;
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
            }
        }
    }

    std::string finalResult = vosk_recognizer_final_result(recognizer);
    if (callback) callback(finalResult);

    pclose(pipe);
    vosk_recognizer_free(recognizer);
    recognizer = nullptr;
}

void SpeechRecognizer::setGrammar(const std::vector<std::string>& commands) {
    if (!recognizer) {
        std::cerr << "[SpeechRecognizer] recognizer is null, skip setGrammar()\n";
        return;
    }

    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < commands.size(); ++i) {
        oss << "\"" << commands[i] << "\"";
        if (i != commands.size() - 1)
            oss << ", ";
    }
    oss << "]";
    std::string grammarJson = oss.str();

    vosk_recognizer_reset(recognizer);
    vosk_recognizer_set_grm(recognizer, grammarJson.c_str());

    std::cout << "[SpeechRecognizer] Grammar updated: " << grammarJson << std::endl;
}
