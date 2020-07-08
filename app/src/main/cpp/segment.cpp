//
//  segment.cpp
//  MNN
//
//  Created by MNN on 2019/07/01.
//  Copyright © 2018, Alibaba Group Holding Limited
//

#include <stdio.h>
#include <ImageProcess.hpp>
#define MNN_OPEN_TIME_TRACE
#include <algorithm>
#include <fstream>
#include <functional>
#include <memory>
#include <sstream>
#include <vector>
//#include <expr/Expr.hpp>
//#include <expr/ExprCreator.hpp>
#include <AutoTime.hpp>
#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"
#include <net.h>
//using namespace MNN;
using namespace MNN::CV;
//using namespace MNN::Express;

int main(int argc, const char* argv[]) {
    if (argc < 4) {
        MNN_PRINT("Usage: ./segment.out model.mnn input.jpg output.jpg\n");
        return 0;
    }
    
    auto netPtr = MNN::Interpreter::createFromFile(argv[1]);
    if (nullptr == netPtr) return -1;

    MNN::ScheduleConfig sch_config;
    sch_config.type = (MNNForwardType)MNN_FORWARD_CPU;

    MNN::BackendConfig backendConfig;

    backendConfig.precision = (MNN::BackendConfig::PrecisionMode)2;
    sch_config.backendConfig = &backendConfig;

    auto sessionPtr = netPtr->createSession(sch_config);
    if (nullptr == sessionPtr) return -1;
    return 0;
}
