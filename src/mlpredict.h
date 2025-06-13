#pragma once 

#include <string>
#include <vector>
#include <memory> 
#include <onnxruntime_cxx_api.h> 

struct PredictionResult {
    int64_t label; 
    std::vector<float> probabilities; 
};


class MLPredictor {
public:
    MLPredictor(const std::string& model_path);

    ~MLPredictor() = default;

    PredictionResult predict(const std::vector<float>& input_data);

private:
    Ort::Env env;
    Ort::SessionOptions session_options;
    std::unique_ptr<Ort::Session> session; 
    
    std::string input_name;
    std::vector<int64_t> input_shape;
    std::vector<const char*> output_names_char;
    std::vector<std::string> output_names_str;
};