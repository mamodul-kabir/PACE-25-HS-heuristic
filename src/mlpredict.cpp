#include "mlpredict.h"
#include <stdexcept>
#include <iostream>
#include <algorithm> // For std::max_element

MLPredictor::MLPredictor(const std::string& model_path)
    : env(ORT_LOGGING_LEVEL_WARNING, "MLPredictor") {

    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    session = std::make_unique<Ort::Session>(env, model_path.c_str(), session_options);

    Ort::AllocatorWithDefaultOptions allocator;
    input_name = session->GetInputNameAllocated(0, allocator).get();
    auto type_info = session->GetInputTypeInfo(0);
    auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
    input_shape = tensor_info.GetShape();
    
    if (input_shape.size() < 2 || input_shape[0] != -1) {
        std::cerr << "Warning: Model input shape may not be dynamic. Expected [-1, num_features]." << std::endl;
    }
    size_t num_output_nodes = session->GetOutputCount();
    for (size_t i = 0; i < num_output_nodes; i++) {
        output_names_str.push_back(session->GetOutputNameAllocated(i, allocator).get());
    }
    for (const auto& name : output_names_str) {
        output_names_char.push_back(name.c_str());
    }
}

PredictionResult MLPredictor::predict(const std::vector<float>& input_data) {
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<int64_t> input_tensor_dims = {1, (int64_t)input_data.size()};

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        const_cast<float*>(input_data.data()),
        input_data.size(),
        input_tensor_dims.data(),
        input_tensor_dims.size()
    );
    const char* input_names_c_str[] = {input_name.c_str()};
    auto output_tensors = session->Run(
        Ort::RunOptions{nullptr},
        input_names_c_str, 
        &input_tensor, 1,
        output_names_char.data(), output_names_char.size()
    );
    PredictionResult result;
    if (output_tensors.size() < 1 || !output_tensors[0].IsTensor() ||
        output_tensors[0].GetTensorTypeAndShapeInfo().GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64) {
        throw std::runtime_error("First output is not a valid int64_t tensor for label.");
    }
    int64_t* label_data = output_tensors[0].GetTensorMutableData<int64_t>();
    result.label = label_data[0]; 

    if (output_tensors.size() < 2 || !output_tensors[1].IsTensor() ||
        output_tensors[1].GetTensorTypeAndShapeInfo().GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
        throw std::runtime_error("Second output is not a valid float tensor for probabilities.");
    }
    float* proba_data = output_tensors[1].GetTensorMutableData<float>();
    auto proba_info = output_tensors[1].GetTensorTypeAndShapeInfo();

    if (proba_info.GetShape().size() != 2 || proba_info.GetShape()[0] != 1) {
        throw std::runtime_error("Probability tensor has unexpected shape. Expected [1, num_classes].");
    }
    size_t num_classes = proba_info.GetShape()[1]; 

    result.probabilities.assign(proba_data, proba_data + num_classes);

    return result;
}