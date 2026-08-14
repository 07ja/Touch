#include <iostream>

#include "onnxModel.hpp"


ONNXModel::ONNXModel(const char* modelPath, const char* inputName, const char* outputName)
    : env(ORT_LOGGING_LEVEL_WARNING, "ONNX"), memoryInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)), inputName(inputName), outputName(outputName) {
    
    // Configure ONNX runtime settings
    Ort::SessionOptions options;
    options.SetIntraOpNumThreads(1);

    try {
        session = Ort::Session(env, modelPath, options);
        std::cout << "Loaded model: " << modelPath << std::endl;
    }

    catch (const Ort::Exception& e) {
        std::cerr << "Failed to load model: " << e.what() << std::endl;
    }
}

std::vector<float> ONNXModel::run(const std::vector<float>& input, const std::vector<int64_t>& shape) {
    // Create input tensor
    Ort::Value tensor = Ort::Value::CreateTensor<float>(memoryInfo, const_cast<float*>(input.data()), input.size(), shape.data(), shape.size());

    const char* inputNode = inputName.c_str();
    const char* outputNode = outputName.c_str();

    // Run model inference
    auto output = session.Run(Ort::RunOptions{nullptr}, &inputNode, &tensor, 1, &outputNode, 1);
    float* data = output[0].GetTensorMutableData<float>();

    auto outputShape = output[0].GetTensorTypeAndShapeInfo().GetShape();

    // Calculate the output size
    int size = 1;
    for (auto dimension : outputShape) {
        size *= dimension;
    }

    return std::vector<float>(data, data + size);
}