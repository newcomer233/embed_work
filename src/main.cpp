#include "PiperSynthesizer.h"

int main() {
    std::string modelPath = "/home/newcomer233/Desktop/piper-master/piper_models/en_US-amy-medium.onnx";
    std::string configPath = modelPath + ".json";
    std::string espeakPath = "/usr/lib/aarch64-linux-gnu/espeak-ng-data";
    std::string outputPath = "/home/newcomer233/output.wav";

    PiperSynthesizer synth(modelPath, configPath, espeakPath);

    synth.synthesizeTextToFile("Hello from the modular Piper wrapper!", outputPath);
    synth.playAudioFile(outputPath);

    return 0;
}
