#pragma once

#include <vector>
#include <string>
#include <onnxruntime_cxx_api.h>

class ONNXModel {
    private:
        Ort::Env env;
        Ort::Session session{nullptr};
        Ort::MemoryInfo memoryInfo{nullptr};

        std::string inputName;
        std::string outputName;

    public:
        ONNXModel(const char* modelPath, const char* inputName, const char* outputName);

        std::vector<float> run(const std::vector<float>& input, const std::vector<int64_t>& shape);
};