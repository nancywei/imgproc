#include "net.h"
#define TAG "net"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
Inference_engine::Inference_engine()
{ }

Inference_engine::~Inference_engine()
{ 
    if ( netPtr != NULL )
	{
		if ( sessionPtr != NULL)
		{
			netPtr->releaseSession(sessionPtr);
			sessionPtr = NULL;
		}

		delete netPtr;
		netPtr = NULL;
	}
}
//create session for body segment
int Inference_engine::load_map(std::string & file)
{
  auto net = MNN::Express::Variable::getInputAndOutput(MNN::Express::Variable::loadMap(file.c_str()));
    if (net.first.empty()) {
        MNN_ERROR("Invalid Model\n");
        return 0;
    }

  auto input = net.first.begin()->second;
    auto info = input->getInfo();
    if (nullptr == info) {
        MNN_ERROR("The model don't have init dim\n");
        return 0;
    }
    auto shape = input->getInfo()->dim;
    shape[0]   = 1;
    input->resize(shape);
  
}

int Inference_engine::load_param(std::string & file, int num_thread)
{

    if (!file.empty())
    {

        if (file.find(".mnn") != std::string::npos)
        {

	        netPtr = MNN::Interpreter::createFromFile(file.c_str());
            if (nullptr == netPtr) return -1;

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

int Inference_engine::set_params(int srcType, int dstType,
                                 float *mean, float *normals)
{
   /* config.destFormat   = (MNN::CV::ImageFormat)dstType;
    config.sourceFormat = (MNN::CV::ImageFormat)srcType;

    ::memcpy(config.mean,   mean,   3 * sizeof(float));
    ::memcpy(config.normal, scale,  3 * sizeof(float));

    config.filterType = (MNN::CV::Filter)(1);
    config.wrap = (MNN::CV::Wrap)(2);*/
    config.filterType = MNN::CV::NEAREST;
    //        float mean[3]     = {103.94f, 116.78f, 123.68f};
    //        float normals[3] = {0.017f, 0.017f, 0.017f};
    //mean[3]     = {127.5f, 127.5f, 127.5f};
    //float normals[3] = {0.00785f, 0.00785f, 0.00785f};
    ::memcpy(config.mean, mean, sizeof(mean));
    ::memcpy(config.normal, normals, sizeof(normals));
    config.sourceFormat = MNN::CV::RGBA;
    config.destFormat   = MNN::CV::RGB;

    tensorPtr = netPtr->getSessionInput(sessionPtr, nullptr);
    tensorOutPtr = netPtr->getSessionOutput(sessionPtr, nullptr);
    nhwcTensor = new MNN::Tensor(tensorOutPtr, MNN::Tensor::TENSORFLOW);

    int dstw = 257;
    int dsth = 257;
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

// infer
//int Inference_engine::infer_img(unsigned char *data, int width, int height, int channel, int dstw, int dsth, Inference_engine_tensor& out)
int Inference_engine::infer_img(unsigned char *data, int width, int height, int channel, int dstw, int dsth, unsigned char* output_pixels)
{

    int h = height;
    int w = width;
    transform.setScale(1.0 / (width - 1.0), 1.0 / (height - 1.0));
    transform.postRotate(90, 0.5, 0.5);
    transform.postScale( (width - 1.0), (height - 1.0));

    process->setMatrix(transform);

    process->convert(data, w, h, w*4, tensorPtr);
    netPtr->runSession(sessionPtr);


        std::vector<int> shape = tensorOutPtr->shape();


//        auto nhwcTensor = new MNN::Tensor(tensorOutPtr, MNN::Tensor::TENSORFLOW);
        tensorOutPtr->copyToHostTensor(nhwcTensor);

        int ch = 21;

        auto outputHostPtr = nhwcTensor->host<float>();

        int total = height * width;

        uint8_t* wrap = ( uint8_t* )malloc( total * 4 * sizeof(uint8_t));
        memset(wrap,0,(total<<2) * sizeof(uint8_t));

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
                        rgba[0] = 0;
                        break;
                    }
                }
                if (index == 15)   rgba[0] = 255;

            }

        }


    memcpy(output_pixels,wrap,4 * dstw * dsth * sizeof(uint8_t));
    free(wrap);


    return 0;
}
