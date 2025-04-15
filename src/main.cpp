#include "vosk_api.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>

int main() {
    // 加载模型，路径为相对于程序运行目录
    VoskModel *model = vosk_model_new("../../model");
    if (!model) {
        std::cerr << "❌ Failed to load model from ../../model" << std::endl;
        return 1;
    }

    VoskRecognizer *recognizer = vosk_recognizer_new(model, 16000.0f);
    if (!recognizer) {
        std::cerr << "❌ Failed to create recognizer" << std::endl;
        vosk_model_free(model);
        return 1;
    }

    std::cout << "🎤 Listening... Say something!\n";

    // 打开 arecord 子进程，用于实时采集麦克风音频
    FILE *pipe = popen("arecord -q -f S16_LE -r 16000 -c 1", "r");
    if (!pipe) {
        std::cerr << "❌ Failed to open arecord pipe\n";
        vosk_recognizer_free(recognizer);
        vosk_model_free(model);
        return 1;
    }

    char buffer[4096];
    while (!feof(pipe)) {
        size_t nread = fread(buffer, 1, sizeof(buffer), pipe);
        if (vosk_recognizer_accept_waveform(recognizer, buffer, nread)) {
            std::cout << "Result: " << vosk_recognizer_result(recognizer) << std::endl;
        } else {
            std::cout << "⏳ Partial: " << vosk_recognizer_partial_result(recognizer) << std::endl;
        }
    }

    std::cout << "🟢 Final: " << vosk_recognizer_final_result(recognizer) << std::endl;

    pclose(pipe);
    vosk_recognizer_free(recognizer);
    vosk_model_free(model);
    return 0;
}
