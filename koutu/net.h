#ifndef _NET_H_
#define _NET_H_

#include <vector>
#include <string>
#include <ImageProcess.hpp>
#include <Interpreter.hpp>
#include <Tensor.hpp>
#include <MNN/expr/Expr.hpp>
#include <MNN/expr/ExprCreator.hpp>
#include <memory>

class Inference_engine_tensor
{
public:
    Inference_engine_tensor()
    { }

    ~Inference_engine_tensor()
    { }

    void add_name(std::string &layer)
    {
        layer_name.push_back(layer);
    }
    std::shared_ptr<float> score(int idx)
    {
        return out_feat[idx];
    }

public:
    std::vector<std::string> layer_name;
    std::vector<std::shared_ptr<float>> out_feat;
};

class Inference_engine
{
public:
    Inference_engine();
    ~Inference_engine();

    int load_param(std::string &file, int num_thread = 1);
    int load_map(std::string &file);
    int set_params(int inType, int outType, float *mean, float *scale);
 //   int infer_img(unsigned char* data, int w, int h, int channel, int dstw, int dsth, Inference_engine_tensor& out);
    int infer_img(unsigned char* data, int w, int h, int channel, int dstw, int dsth, unsigned char* output);

private: 
    MNN::Interpreter* netPtr;
	MNN::Session* sessionPtr;
    MNN::CV::ImageProcess::Config config;
    MNN::Tensor* tensorPtr ;  //= netPtr->getSessionInput(sessionPtr, nullptr);
    MNN::Tensor* tensorOutPtr;// = netPtr->getSessionOutput(sessionPtr, nullptr);
    MNN::CV::ImageProcess* process;//(MNN::CV::ImageProcess::create(config));
    MNN::CV::Matrix transform;

    MNN::Tensor* nhwcTensor;
    // std::shared_ptr<MNN::Tensor> wrapTensor(MNN::CV::ImageProcess::createImageTensor<uint8_t>(1280, 720, 4, nullptr));

};
#endif
