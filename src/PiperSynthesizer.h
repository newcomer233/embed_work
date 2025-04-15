#ifndef PIPER_SYNTHESIZER_H
#define PIPER_SYNTHESIZER_H

#include <string>
#include "piper.hpp"

class PiperSynthesizer {
public:
    PiperSynthesizer(const std::string& modelPath,
                     const std::string& configPath,
                     const std::string& espeakPath);

    ~PiperSynthesizer();

    void synthesizeTextToFile(const std::string& text, const std::string& outPath);
    void playAudioFile(const std::string& path);

private:
    piper::PiperConfig piperConfig;
    piper::Voice voice;
};

#endif // PIPER_SYNTHESIZER_H
