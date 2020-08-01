//
//  segment.cpp
//  MNN
//
//  Created by MNN on 2019/07/01.
//  Copyright © 2018, Alibaba Group Holding Limited
//

#include <stdio.h>
#include <MNN/ImageProcess.hpp>
#define MNN_OPEN_TIME_TRACE
#include <algorithm>
#include <fstream>
#include <functional>
#include <memory>
#include <sstream>
#include <vector>
#include <net.h>
#include <MNN/expr/Expr.hpp>
#include <MNN/expr/ExprCreator.hpp>
#include <MNN/AutoTime.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

using namespace MNN;
using namespace MNN::CV;
using namespace MNN::Express;

int main(int argc, const char* argv[]) {
    if (argc < 4) {
        MNN_PRINT("Usage: ./segment.out model.mnn input.jpg output.jpg\n");
        return 0;
    }

    std::string s(argv[1]);
    Inference_engine *infer = new Inference_engine();
    int result = infer->load_param(s,4);
    printf("init result %d \n",result);
    
    
// start set params
    float mean[3]     = {127.5f, 127.5f, 127.5f};
    float normals[3] = {0.00785f, 0.00785f, 0.00785f};

    int sourceFormat = RGBA;
    int destFormat   = RGB;

    infer->set_params(sourceFormat, destFormat,&mean[0],&normals[0]);
// end set params

//start prepare input
   auto inputPatch = argv[2];
   int width, height, channel;
   auto inputImage = stbi_load(inputPatch, &width, &height, &channel, 4);
   if (nullptr == inputImage) {
            MNN_ERROR("Can't open %s\n", inputPatch);
            return 0;
   }
   MNN_PRINT("origin size: %d, %d, %d\n", width, height, channel);
// end of prepare input
   int size_w = 257;
   int size_h = 257;
   int c = 3;

   unsigned char* output_pixels = (unsigned char*)malloc(sizeof(unsigned char) * size_w * size_h * 4);
  
   infer->infer_img(inputImage, width, height, c, size_w, size_h,output_pixels);

   stbi_write_png(argv[3], size_w, size_h, 4, output_pixels, 4 * size_w);

   free(output_pixels);
   return 0;
}

