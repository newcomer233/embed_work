#include "PiperSynthesizer.h"
#include <fstream>
#include <iostream>
#include <cstdlib> // for system()
#include "json.hpp"

PiperSynthesizer::PiperSynthesizer(const std::string& modelPath,
                                   const std::string& configPath,
                                   const std::string& espeakPath) {
    piperConfig.eSpeakDataPath = espeakPath;

    std::optional<piper::SpeakerId> speakerId;
    loadVoice(piperConfig, modelPath, configPath, voice, speakerId, false);
    piper::initialize(piperConfig);
}

PiperSynthesizer::~PiperSynthesizer() {
    piper::terminate(piperConfig);
}

void PiperSynthesizer::synthesizeTextToFile(const std::string& text, const std::string& outPath) {
    std::ofstream audioFile(outPath, std::ios::binary);
    if (!audioFile) {
        std::cerr << "ERROR: Failed to open output file." << std::endl;
        return;
    }

    piper::SynthesisResult result;
    piper::textToWavFile(piperConfig, voice, text, audioFile, result);
    audioFile.close();

    std::ifstream check(outPath, std::ios::binary | std::ios::ate);
    if (check.tellg() < 10000) {
        std::cerr << "ERROR: Output file is too small." << std::endl;
    } else {
        std::cout << "✅ Synthesis OK. Output: " << outPath << std::endl;
    }
}

void PiperSynthesizer::playAudioFile(const std::string& path) {
    std::string cmd = "aplay \"" + path + "\"";
    system(cmd.c_str());
}
