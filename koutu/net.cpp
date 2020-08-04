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
                                 float *mean, float *scale)
{
    config.destFormat   = (MNN::CV::ImageFormat)dstType;
    config.sourceFormat = (MNN::CV::ImageFormat)srcType;

    ::memcpy(config.mean,   mean,   3 * sizeof(float));
    ::memcpy(config.normal, scale,  3 * sizeof(float));

    config.filterType = (MNN::CV::Filter)(1);
    config.wrap = (MNN::CV::Wrap)(2);

    return 0;
}

// infer
//int Inference_engine::infer_img(unsigned char *data, int width, int height, int channel, int dstw, int dsth, Inference_engine_tensor& out)
int Inference_engine::infer_img(unsigned char *data, int width, int height, int channel, int dstw, int dsth, unsigned char* output_pixels)
{
    MNN::Tensor* tensorPtr = netPtr->getSessionInput(sessionPtr, nullptr);
    MNN::CV::Matrix transform;

    int h = height;
    int w = width;
    int c = channel;
    int size_w = dstw;
    int size_h = dsth;
    // auto resize for full conv network.
    bool auto_resize = false;
    if ( !auto_resize )
    {
        std::vector<int>dims = { 1, c, dsth, dstw };
        netPtr->resizeTensor(tensorPtr, dims);
        netPtr->resizeSession(sessionPtr);
    }

   // transform.postScale(1.0f/dstw, 1.0f/dsth);
  //  transform.postScale(w, h);

       config.filterType = MNN::CV::BILINEAR;
        //        float mean[3]     = {103.94f, 116.78f, 123.68f};
        //        float normals[3] = {0.017f, 0.017f, 0.017f};
        float mean[3]     = {127.5f, 127.5f, 127.5f};
        float normals[3] = {0.00785f, 0.00785f, 0.00785f};
        ::memcpy(config.mean, mean, sizeof(mean));
        ::memcpy(config.normal, normals, sizeof(normals));
        config.sourceFormat = MNN::CV::RGBA;
        config.destFormat   = MNN::CV::RGB;


    transform.setScale((float)(width-1) / (dstw-1), (float)(height-1) / (dsth-1));
    std::unique_ptr<MNN::CV::ImageProcess> process(MNN::CV::ImageProcess::create(config));

    process->setMatrix(transform);
  //  process->convert((uint8_t*)data, width, height, 0, tensorPtr, size_w, size_h, 4, 0, halide_type_of<float>());
      
    process->convert(data, w, h, w*4, tensorPtr);
    netPtr->runSession(sessionPtr);
#if 0
    for (int i = 0; i < out.layer_name.size(); i++)
    {
        const char* layer_name = NULL;
        if( strcmp(out.layer_name[i].c_str(), "") != 0)
        {
            layer_name = out.layer_name[i].c_str();
        }
#endif
       const char* layer_name = NULL;
       
        MNN::Tensor* tensorOutPtr = netPtr->getSessionOutput(sessionPtr, layer_name);

        std::vector<int> shape = tensorOutPtr->shape();

        auto tensor = reinterpret_cast<MNN::Tensor*>(tensorOutPtr);

        std::unique_ptr<MNN::Tensor> hostTensor(new MNN::Tensor(tensor, tensor->getDimensionType(), true));     
        auto size = tensorOutPtr->elementSize();
 //       printf("dest element size %d\n",size);


        tensor->copyToHostTensor(hostTensor.get());
        tensor = hostTensor.get();

        auto nhwcTensor = new MNN::Tensor(tensorOutPtr, MNN::Tensor::TENSORFLOW);
        tensorOutPtr->copyToHostTensor(nhwcTensor);
        auto score = nhwcTensor->host<float>();
        //auto index = nhwcTensor->host<float>();
   /*   for ( int j = 0; j < 257; j++){
        float * s= score + j * 257 * 21;
        for ( int i = 0;i < 257;i++){
            printf("num %d: %0.4f ;\n",i, *(s + 21 * i ));
        }
   }*/ 
       // std::shared_ptr<float> destPtr(new float[size * sizeof(float)]);

      //  ::memcpy(destPtr.get(), tensorOutPtr->host<float>(), size * sizeof(float));

    //    auto output = _Convert(tensorOutPtr, MNN::Express::NHWC);
        //output = _Softmax(output, -1);
  /*      auto outputInfo = output->getInfo();
        auto width = outputInfo->dim[2];
        auto height = outputInfo->dim[1];
        auto channel = outputInfo->dim[3];
    */
       int ch = 21;
        std::shared_ptr<MNN::Tensor> wrapTensor(MNN::CV::ImageProcess::createImageTensor<uint8_t>(width, height, 4, nullptr));
//        MNN_PRINT("Mask: w=%d, h=%d, index=%d\n", width, height, index);
        auto outputHostPtr = nhwcTensor->host<float>();
        int ch2 = 22; // ch + 1 for caculate sourceX[0];
        int total = height * width;
        memset(wrapTensor->host<uint8_t>(),0,(total<<2) * sizeof(uint8_t));

        for (int i = 0; i < total; ++i) {

            float* sourceX =  outputHostPtr + ch * i;
            uint8_t* rgba = wrapTensor->host<uint8_t>() + 4 * i;
            int index = 15;

            if (sourceX[0] >= sourceX[15]) {
                rgba[0] = 0;
            }
            else{

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
#if 0
        for (int y = 0; y < height; ++y) {
            auto rgbaY = wrapTensor->host<uint8_t>() + 4 * y * width;
            auto sourceY =  outputHostPtr + y * width * ch;
            for (int x=0; x<width; ++x) {
                auto sourceX = sourceY + ch * x;
                int index = 0;
                float maxValue = sourceX[0];
                auto rgba = rgbaY + 4 * x;
                for (int c=1; c<ch; ++c) {
                    if (sourceX[c] > maxValue) {
                        index = c;
                        maxValue = sourceX[c];
                    }
                }
                rgba[0] = 0;
                rgba[2] = 0;
                rgba[1] = 0;
                rgba[3] = 255;
                if (15 == index) {
                    rgba[0] = 255;
                    rgba[1] = 255;
                    rgba[2] = 255;
                    rgba[3] = 0;
                }
            }
        }
#endif
   //     unsigned char * out = output_pixels;
   //     for (int j = 0 ;j < dsth; j ++ ){
   //         out = output_pixels + j * dstw * 4;
            memcpy(output_pixels,wrapTensor->host<uint8_t>(),4 * dstw * dsth * sizeof(uint8_t));
     //   }
 //       stbi_write_png("./output.png", dstw, dsth, 4, wrapTensor->host<uint8_t>(), 4 * dstw);
//        output->unMap();
#if 0

        out.out_feat.push_back(destPtr);
    }
#endif

    return 0;
}
