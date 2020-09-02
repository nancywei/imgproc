#include "net.h"
#define TAG "net"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)


namespace trinity {

Inference_engine::Inference_engine()
{ }

Inference_engine::~Inference_engine()
{ 
    if (netPtr != nullptr && sessionPtr != nullptr)
    {
        netPtr->releaseSession(sessionPtr);
        netPtr->releaseModel();

        sessionPtr = nullptr;

        delete netPtr;
        netPtr = nullptr;
    }

    if ( netPtr != nullptr )
    {
        netPtr->releaseModel();

		delete netPtr;
		netPtr = nullptr;
	}
}


int Inference_engine::load_param(const char* config_path, int num_thread)
{

    const char * mnn_path = "/deep.mnn";

    auto* model_path = new char[strlen(config_path) + strlen(mnn_path)];
    sprintf(model_path, "%s%s", config_path, mnn_path);

    std::string file(model_path);

    if (!file.empty())
    {

        if (file.find(".mnn") != std::string::npos)
        {

	        netPtr = MNN::Interpreter::createFromFile(file.c_str());
            if (nullptr == netPtr)
                return -1;

            MNN::ScheduleConfig sch_config;
            sch_config.type = (MNNForwardType)MNN_FORWARD_OPENGL;
           // sch_config.type = (MNNForwardType)MNN_FORWARD_CPU;
            if ( num_thread > 0 )sch_config.numThread = num_thread;

            MNN::BackendConfig backendConfig;

            backendConfig.precision = (MNN::BackendConfig::PrecisionMode)2;
            sch_config.backendConfig = &backendConfig;

            sessionPtr = netPtr->createSession(sch_config);
            if (nullptr == sessionPtr) return -1;
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

int Inference_engine::set_resize( int width, int height, int dw, int dh) {
        MNN::CV::Matrix pretrans;
        pretrans.setScale((float)(width-1) / (dw-1), (float)(height-1) / (dh-1));

        MNN::CV::ImageProcess::Config preconfig;
        preconfig.filterType   = MNN::CV::BILINEAR;
        preconfig.sourceFormat = MNN::CV::YUV_NV21;
        preconfig.destFormat   = MNN::CV::RGBA;
        preconfig.wrap         = MNN::CV::ZERO;

        //  std::shared_ptr<MNN::CV::ImageProcess> pretreat(MNN::CV::ImageProcess::create(config));
        pretreat = MNN::CV::ImageProcess::create(preconfig);
        pretreat->setMatrix(pretrans);


}


int Inference_engine::get_resize(unsigned char* inputImage, int width, int height, unsigned char* pMask, int dw, int dh) {
    std::shared_ptr<MNN::Tensor> wrapTensor(MNN::CV::ImageProcess::createImageTensor<uint8_t>(dw, dh, 4, nullptr));
    pretreat->convert((uint8_t*)inputImage, width, height, 0, wrapTensor.get());

    memcpy( pMask , wrapTensor->host<uint8_t>() , sizeof(unsigned char) * dw * dh * 4);

}

int Inference_engine::set_params(int width, int height, int dstw, int dsth)
{
    //set_resize( width, height, dstw, dsth);
    pretrans.setScale((float)(width-1) / (dstw-1), (float)(height-1) / (dsth-1));

    preconfig.filterType   = MNN::CV::BILINEAR;
    preconfig.sourceFormat = MNN::CV::YUV_NV21;
    preconfig.destFormat   = MNN::CV::RGBA;
    preconfig.wrap         = MNN::CV::ZERO;

    pretreat = MNN::CV::ImageProcess::create(preconfig);
    pretreat->setMatrix(pretrans);

    //end set preresize
    config.filterType = MNN::CV::NEAREST;
    float mean[3]     = {127.5f, 127.5f, 127.5f};
    float normals[3] = {0.00785f, 0.00785f, 0.00785f};
    ::memcpy(config.mean, mean, sizeof(mean));
    ::memcpy(config.normal, normals, sizeof(normals));
    config.sourceFormat = MNN::CV::RGBA;
    config.destFormat   = MNN::CV::RGB;

    tensorPtr = netPtr->getSessionInput(sessionPtr, nullptr);
    tensorOutPtr = netPtr->getSessionOutput(sessionPtr, nullptr);

   // int dstw = 257;
   // int dsth = 257;
    int c = 3;
    bool auto_resize = false;
    if ( !auto_resize )
    {
        std::vector<int>dims = { 1, c, dsth, dstw };
        netPtr->resizeTensor(tensorPtr, dims);
        netPtr->resizeSession(sessionPtr);
    }
    config.filterType = MNN::CV::NEAREST;
    float mean1[3]     = {127.5f, 127.5f, 127.5f};
    float normals1[3] = {0.00785f, 0.00785f, 0.00785f};
    ::memcpy(config.mean, mean1, sizeof(mean1));
    ::memcpy(config.normal, normals1, sizeof(normals1));
    config.sourceFormat = MNN::CV::RGBA;
    config.destFormat   = MNN::CV::RGB;
    process = MNN::CV::ImageProcess::create(config);

    return 0;
}

// infer in_angle: means rotate degree.
//int Inference_engine::infer_img(unsigned char *data, int width, int height, int channel, int dstw, int dsth, Inference_engine_tensor& out)
int Inference_engine::infer_img(unsigned char *data, int width, int height, int in_angle, int dstw, int dsth, unsigned char* pMask)
{
 //   int h = height;
  //  int w = width;
    float angle = 0.0;

    std::shared_ptr<MNN::Tensor> prewrapTensor(MNN::CV::ImageProcess::createImageTensor<uint8_t>(dstw, dsth, 4, nullptr));
    pretreat->convert((uint8_t*)data, width, height, 0, prewrapTensor.get());

    int total = (dstw * dsth);
    int length = total<<2;  //rgba 4 channel

    uint8_t* wrap = ( uint8_t* )malloc( length * sizeof(uint8_t));
    memset(wrap,0, length* sizeof(uint8_t));

    memcpy( wrap , prewrapTensor->host<uint8_t>() , sizeof(unsigned char) * length);
    //stbi_write_png("/sdcard/data/i.png", 257, 257, 4, prewrapTensor->host<uint8_t>(), 4 * 257);

#if 1
    if (in_angle != 0.) angle = 360.0 - in_angle ;
    transform.setScale(1.0 / (dstw - 1.0), 1.0 / (dsth - 1.0));
    transform.postRotate(angle, 0.5, 0.5);
    transform.postScale( (dstw - 1.0), (dsth - 1.0));

    process->setMatrix(transform);

    process->convert(wrap, dstw, dsth, dstw*4, tensorPtr);


        netPtr->runSession(sessionPtr);
        std::vector<int> shape = tensorOutPtr->shape();


        auto nhwcTensor = new MNN::Tensor(tensorOutPtr, MNN::Tensor::TENSORFLOW);
        tensorOutPtr->copyToHostTensor(nhwcTensor);

        int ch = 21;

        auto outputHostPtr = nhwcTensor->host<float>();
        memset(wrap,0, length* sizeof(uint8_t));

        for (int i = 0; i < total; ++i) {

            float* sourceX =  outputHostPtr + ch * i;
    //        uint8_t* rgba = wrapTensor->host<uint8_t>() + 4 * i;
            uint8_t* rgba = wrap + 4 * i;
            int index = 15;

            if (sourceX[0] < sourceX[15]) {
      
                float maxValue = sourceX[15];
                for (int c=1; c<ch && c!=15; ++c) {
                    if (sourceX[c] > maxValue) {
                        index = c;
              //          maxValue = sourceX[c];
               //         rgba[0] = 0;
                        break;
                    }
                }
                if (index == 15)  {
                    rgba[0] = 255;
                    rgba[1] = 255;
                    rgba[2] = 255;
                    rgba[3] = 255;
                }

            }

        }


    memcpy(pMask,wrap,4 * dstw * dsth * sizeof(uint8_t));
        delete nhwcTensor;

#endif
    free(wrap);

//#endif
    return 0;
}

/*
Inference_engine::~Inference_engine(){
  netPtr->releaseSession(sessionPtr);

}*/

}