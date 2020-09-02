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

namespace trinity {

class Inference_engine
{
public:
    Inference_engine();
    ~Inference_engine();

    MNN::Session* sessionPtr = nullptr;
    int load_param(const char* mnn_path, int num_thread = 1);
    int load_map(std::string &file);
    int set_params(int width, int height, int dstw, int dsth);
    int set_resize( int width, int height, int dw, int dh);
    int get_resize(unsigned char* inputImage, int width, int height, unsigned char* pMask, int dw, int dh);
  //  int set_params(int inType, int outType, float *mean, float *scale);
 //   int infer_img(unsigned char* data, int w, int h, int channel, int dstw, int dsth, Inference_engine_tensor& out);
    int infer_img(unsigned char* data, int w, int h, int channel, int dstw, int dsth, unsigned char* output);

  private:
    MNN::Interpreter* netPtr;
    MNN::CV::ImageProcess::Config preconfig;
    MNN::CV::Matrix pretrans;
    MNN::CV::ImageProcess* pretreat;

    MNN::CV::ImageProcess::Config config;
    MNN::Tensor* tensorPtr ;  //= netPtr->getSessionInput(sessionPtr, nullptr);
    MNN::Tensor* tensorOutPtr;// = netPtr->getSessionOutput(sessionPtr, nullptr);
    MNN::CV::ImageProcess* process;//(MNN::CV::ImageProcess::create(config));
    MNN::CV::Matrix transform;

    MNN::Tensor* nhwcTensor;
    // std::shared_ptr<MNN::Tensor> wrapTensor(MNN::CV::ImageProcess::createImageTensor<uint8_t>(1280, 720, 4, nullptr));

};
}
#endif
