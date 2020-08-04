/*
 * Copyright (C) 2019 Trinity. All rights reserved.
 * Copyright (C) 2019 Wang LianJie <wlanjie888@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// Created by wlanjie on 2019-06-05.
//

#include "image_process.h"
#include "image_buffer.h"
#include <utility>

#if __ANDROID__
#include "android_xlog.h"
#elif __APPLE__
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#define LOGE(format, ...) fprintf(stdout, format, __VA_ARGS__) // NOLINT
#define LOGI(format, ...) fprintf(stdout, format, __VA_ARGS__) // NOLINT
#endif

namespace trinity {


ImageProcess::ImageProcess() 
    : action_id_(-1) {}
   

ImageProcess::~ImageProcess() {
    ClearAction();
    infer->~Inference_engine();
}

GLuint ImageProcess::Process(GLuint texture_id, int64_t current_time, int width, int height, int input_color_type, int output_color_type) {
    return OnProcess(texture_id, current_time, width, height);
}

int ImageProcess::Process(uint8_t *frame, uint64_t current_time, int width, int height, int input_color_type, int output_color_type) {
    return OnProcess(0, current_time, width, height);
}

GLuint ImageProcess::OnProcess(GLuint texture_id, int64_t current_time, int width, int height) {
    GLuint texture = texture_id;
    //GLuint pre_texture = texture_id;
    // 执行滤镜操作

    for (auto& filter : filters_) {
        int ct = 0;
        float enableMask = 0.;
        Filter *f = filter.second;

#if 1
        f->ActiveProgram();
        f->SetEnableMask(enableMask);
        GLuint pre_texture = f->OnDrawFrame(texture, ct);

        f->ProcessImage(pre_texture);

        // int width = width;
        //   int height = height;
        //   unsigned char* output_pixels = (unsigned char*)malloc(sizeof(unsigned char) * out_w * out_h * 4);
        if (frame_status == false) {

    //    unsigned char *outbuffer = (unsigned char *) malloc(
     //           sizeof(unsigned char) * width * height * 4);
     //   unsigned char *pMask = (unsigned char *) malloc(sizeof(unsigned char) * 257 * 257 * 4);
        //unsigned char *pResult = (unsigned char *) malloc(sizeof(unsigned char) * 257 * 257 * 4);
        int dw = 257;

        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pbuffer_);
        stbir_resize_uint8(pbuffer_, width, height, 0, pmask512_, dw, dw, 0, 4);
        //   stbi_write_png("/sdcard/data/bi.png", dw, dw, 4, pMask, dw * 4);

        f->GetRotate(pmask512_, dw, dw, 4, pResult_);
        //      stbi_write_png("/sdcard/data/i.png", dw, dw, 4, pResult, dw * 4);


        //unsigned char* outY = outbuffer;
        //     infer->infer_img(pResult, width, height, 3, dw, dw, pMask);
        // int dw = 257;
        //  unsigned char* output_pixels = (unsigned char*)malloc(sizeof(unsigned char) * dw * dw * 4);
        int c = 3;
       // int dw = 257;

        infer->infer_img(pResult_, dw, dw, c, dw, dw, pmask512_);

        //   stbi_write_png("/sdcard/data/o.png", dw, dw, 4, pMask, 4 * dw);

//    free(output_pixels);
        // f->GetPortrait( pResult, dw, dw , pMask );
        //stbi_write_png("/sdcard/data/o.png", dw, dw, 4, pMask, dw * 4);
        f->GetRotate(pmask512_, dw, dw, 4, pResult_);

        stbir_resize_uint8(pResult_, dw, dw, 0, pmask_, 512, 512, 0, 4);
        //       stbi_write_png("/sdcard/data/r.png", 512, 512, 4, pmask_, 512 * 4);
        //free(outbuffer);
        //free(pMask);
        //free(pResult);
        f->UpdateLut(pmask_, 512, 512);

            frame_status = true;
        } else{
            frame_status = false;

        }
    //    memset(pmask_, 1, sizeof(unsigned char) * 512 * 512 * 4);


        //     if(has_face) f->SetPosition(face_keypoints);
        //     stbi_write_png("/sdcard/data/p.png", 512, 512, 4, pmask_, 512 * 4);
//}
#endif

#if 0
        int c = 3;
        int dw = 257;
   //     unsigned char *pResult = (unsigned char *) malloc(sizeof(unsigned char) * 512 * 512 * 4);

        infer->infer_img(pmask_, dw, dw, c, dw, dw, pmask257_);
        stbir_resize_uint8(pmask257_, dw, dw, 0, pResult_, 512, 512, 0, 4);

        f->UpdateLut(pResult_, 512, 512);
     //   free(pResult);
#endif

        enableMask = 1.0;
        f->SetEnableMask(enableMask);
        GLuint process_texture = f->OnDrawFrame(texture, ct);
        texture = process_texture;


        if (action_id_ == -1) {
            for (auto &effect : effects_) {
                texture = static_cast<GLuint>(effect.second->OnDrawFrame(texture, width, height,
                                                                         current_time));
            }
        } else {
            auto effect_iterator = effects_.find(action_id_);
            if (effect_iterator == effects_.end()) {
                return texture;
            }
            texture = static_cast<GLuint>(effect_iterator->second->OnDrawFrame(texture, width,
                                                                               height,
                                                                               current_time));
        }
    }
    return texture;
}

void ImageProcess::OnAction(char* config_path, int action_id, FaceDetection* face_detection) {
    auto* effect = new Effect();
    effect->SetFaceDetection(face_detection);
    effect->ParseConfig(config_path);
    effects_.insert(std::pair<int, Effect*>(action_id, effect));
    action_id_ = action_id;
}

void ImageProcess::OnUpdateActionTime(int start_time, int end_time, int action_id) {
    LOGI("enter %s start_time: %d end_time: %d action_id: %d", __func__, start_time, end_time, action_id);
    if (!effects_.empty() && action_id != -1) {
        auto effect_iterator = effects_.find(action_id);
        if (effect_iterator != effects_.end()) {
            effect_iterator->second->UpdateTime(start_time, end_time);
        }
    }
    action_id_ = -1;
    LOGI("leave %s", __func__);
}

void ImageProcess::OnUpdateEffectParam(int action_id, const char *effect_name, const char *param_name, float value) {
    if (!effects_.empty() && action_id != -1) {
        auto effect_iterator = effects_.find(action_id);
        if (effect_iterator != effects_.end()) {
            effect_iterator->second->UpdateParam(effect_name, param_name, value);
        }
    }
}

void ImageProcess::OnUpdateEffectParam(int action_id, const char *effect_name, const char *param_name, float *value, int length) {
    if (!effects_.empty() && action_id != -1) {
        auto effect_iterator = effects_.find(action_id);
        if (effect_iterator != effects_.end()) {
            effect_iterator->second->UpdateParam(effect_name, param_name, value, length);
        }
    }
}

int ImageProcess::ReadFile(char *path, char **buffer) {
    FILE *file = fopen(path, "r");
    printf("path: %s\n", path);
    if (file == nullptr) {
        return -1;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    char* data = new char[sizeof(char) * file_size + 1];
    if (nullptr == data) {
        fclose(file);
        return -2;
    }
    memset(data, 0, sizeof(char) * file_size);
    data[file_size] = '\0';
    size_t read_size = fread(data, 1, file_size, file);
    if (read_size != file_size) {
        fclose(file);
        delete[] data;
        return -3;
    }
    fclose(file);
    *buffer = data;
    return 0;
}

void ImageProcess::RemoveAction(int action_id) {
    if (action_id == -1) {
        return;
    }
    if (!effects_.empty()) {
        auto effect_iterator = effects_.find(action_id);
        if (effect_iterator != effects_.end()) {
            Effect *effect = effect_iterator->second;
            delete effect;
            effects_.erase(effect_iterator);
        }
    }
    if (!filters_.empty()) {
        auto filter_iterator = filters_.find(action_id);
        if (filter_iterator != filters_.end()) {
            Filter* filter = filter_iterator->second;
            delete filter;
            filters_.erase(filter_iterator);
        }
    }
}

void ImageProcess::ClearAction() {
    int size = static_cast<int>(effects_.size());
    LOGI("enter: %s action_size: %d", __func__, size);
    auto effect_iterator = effects_.begin();
    while (effect_iterator != effects_.end()) {
        delete effect_iterator->second;
        effect_iterator++;
    }
    effects_.clear();
    for (auto& filter : filters_) {
        delete filter.second;
    }
    filters_.clear();
    LOGI("leave: %s", __func__);
}

void ImageProcess::OnFilter(const char* config_path, int action_id, int start_time, int end_time) {
    char const* config_name = "/config.json";
    auto* config = new char[strlen(config_path) + strlen(config_name)];
    sprintf(config, "%s%s", config_path, config_name);

    char* buffer = nullptr;
    int ret = ReadFile(config, &buffer);
    if (ret != 0 || buffer == nullptr) {
        return;
    }
    cJSON* json = cJSON_Parse(buffer);
    delete[] config;
    delete[] buffer;
    if (nullptr == json) {
        return;
    }
    float intensity = 1.0F;
    cJSON* intensity_json = cJSON_GetObjectItem(json, "intensity");
    if (nullptr != intensity_json) {
        intensity = static_cast<float>(intensity_json->valuedouble);
    }
    cJSON* lut_json = cJSON_GetObjectItem(json, "lut");
    #if 1
   // for mnn body detect init
    const char * mnn_path = "/../../mnn/deep.mnn";

    auto* model_path = new char[strlen(config_path) + strlen(mnn_path)];
    sprintf(model_path, "%s%s", config_path, mnn_path);

    std::string s(model_path);
   
  //  infer = new Inference_engine();
    infer->load_param(s,8);
 
  
 // start set params
    float mean[3]     = {127.5f, 127.5f, 127.5f};
    float normals[3] = {0.00785f, 0.00785f, 0.00785f};

    int sourceFormat = MNN::CV::RGBA;
    int destFormat   = MNN::CV::RGB;

    infer->set_params(sourceFormat, destFormat,&mean[0],&normals[0]);
// end set params
    int width = 0;
    int height = 0;
    int channel = 0;

 //   pbuffer_ = stbi_load("/sdcard/data/body1080.png", &width, &height, &channel, 4);
 //   pResult_ = stbi_load("/sdcard/data/o.png", &width, &height, &channel, 4);


#endif
#if 0
    int width = 0;
    int height = 0;
    int channel = 0;
    auto inputImage = stbi_load("/sdcard/data/i.png", &width, &height, &channel, 4);
    int dw = 257;
    unsigned char* output_pixels = (unsigned char*)malloc(sizeof(unsigned char) * dw * dw * 4);
    int c= 3;
 
    infer->infer_img(inputImage, width, height, c, dw, dw, output_pixels);

    stbi_write_png("/sdcard/data/o.png", dw, dw, 4, output_pixels, 4 * dw);

    free(output_pixels);
#endif
    if (lut_json != nullptr) {
        char* lut_value = lut_json->valuestring;
        auto* lut_path = new char[strlen(config_path) + strlen(lut_value)];
        sprintf(lut_path, "%s/%s", config_path, lut_value);
   //     const char * mnn_path = "/sdcard/data/deep.mnn";
        int lut_width = 0;
        int lut_height = 0;
        int channels = 0;
        unsigned char* lut_buffer = stbi_load(lut_path, &lut_width, &lut_height, &channels, STBI_rgb_alpha);
        delete[] lut_path;

        if (nullptr == lut_buffer) {
            cJSON_Delete(json);
            return;
        }
/*
        char const* fname = "FILTER_FRAGMENT_SHADER" ;
        char const* bname = "BILTER_FRAGMENT_SHADER";
        auto* ftype =  new char[strlen(fname)];*/
     //   auto ftype = 2 ;

        //  unsigned char* cha= "chahui";
      /*  FaceDetection* face_detection;
        std::vector<FaceDetectionReport*> face_detections;
        face_detection->FaceDetector(face_detections);
*/
   /*     float* position = new float[sizeof(float) * 4];
        *position = 0.3;
        *(position + 1) = 0.3;
        *(position + 2) = 0.6;
        *(position + 3) = 0.6;
     */
        int width,height,channel;
       // int out_w = 257;
      //  int out_h = 257;
      //  pmask = stbi_load("/sdcard/data/in.png", &width, &height, &channel,4);
     //   unsigned char* output_pixels = (unsigned char*)malloc(sizeof(unsigned char) * out_w * out_h * 4);

        if ((lut_width == 512 && lut_height == 512) || (lut_width == 64 && lut_height == 64)) {
            auto result = filters_.find(action_id);
            if (result == filters_.end()) {
                // 添加filter
                Filter *filter = NULL;
       //         if (strstr(lut_path,"chahui"))
       //             filter = new Filter(lut_buffer, 720, 1280, 1);
       //         else
                filter = new Filter(lut_buffer, 720, 1280, 1);
                filter->SetStartTime(start_time);
                filter->SetEndTime(end_time);
                filter->SetIntensity(intensity);
         //       filter->SetPortrait( 257,257);
            //    filter->GetPortrait( inputImage ,width ,height, output_pixels ,512 ,512);
             //   stbir_resize_uint8( outbuffer , width , height , 0, pMask, 512, 512, 0, 4);
          //      stbi_write_png("/sdcard/data/output.jpg", 257, 257, 4, output_pixels, 4 * 257);
                //            filter->SetPosition(face_keypoints);
              
                filters_.insert(std::pair<int, Filter*>(action_id,  filter));
            } else {
                // 更新filter
                Filter* filter = filters_[action_id];
                filter->SetStartTime(start_time);
                filter->SetEndTime(end_time);
                filter->SetIntensity(intensity);
    //            filter->SetPosition(face_keypoints);
                
                filter->UpdateLut(lut_buffer, 720, 1280);
            }
        }
        stbi_image_free(lut_buffer);
       // stbi_image_free( inputImage );
        //free(output_pixels);
    }
    cJSON_Delete(json);
}

void ImageProcess::OnAIFilter(const char* config_path, int action_id, int start_time, int end_time,int width,int height) {
    char const* config_name = "/config.json";
    auto* config = new char[strlen(config_path) + strlen(config_name)];
    sprintf(config, "%s%s", config_path, config_name);

    char* buffer = nullptr;
    int ret = ReadFile(config, &buffer);
    if (ret != 0 || buffer == nullptr) {
        return;
    }
    cJSON* json = cJSON_Parse(buffer);
    delete[] config;
    delete[] buffer;
    if (nullptr == json) {
        return;
    }
    float intensity = 1.0F;
    cJSON* intensity_json = cJSON_GetObjectItem(json, "intensity");
    if (nullptr != intensity_json) {
        intensity = static_cast<float>(intensity_json->valuedouble);
    }
    cJSON* lut_json = cJSON_GetObjectItem(json, "lut");
    if (lut_json != nullptr) {
        char* lut_value = lut_json->valuestring;
        auto* lut_path = new char[strlen(config_path) + strlen(lut_value)];
        sprintf(lut_path, "%s/%s", config_path, lut_value);
        const char * mnn_path = "/sdcard/data/deep.mnn";
        int lut_width = 0;
        int lut_height = 0;
        int channels = 0;
        unsigned char* lut_buffer = stbi_load(lut_path, &lut_width, &lut_height, &channels, STBI_rgb_alpha);
        delete[] lut_path;

        if (nullptr == lut_buffer) {
            cJSON_Delete(json);
            return;
        }
/*
        char const* fname = "FILTER_FRAGMENT_SHADER" ;
        char const* bname = "BILTER_FRAGMENT_SHADER";
        auto* ftype =  new char[strlen(fname)];*/
     //   auto ftype = 2 ;

        //  unsigned char* cha= "chahui";
      /*  FaceDetection* face_detection;
        std::vector<FaceDetectionReport*> face_detections;
        face_detection->FaceDetector(face_detections);
*/
   /*     float* position = new float[sizeof(float) * 4];
        *position = 0.3;
        *(position + 1) = 0.3;
        *(position + 2) = 0.6;
        *(position + 3) = 0.6;
     */
        int width,height,channel;
        int out_w = 257; 
        int out_h = 257;
    //    unsigned char* inputImage = stbi_load("/sdcard/data/body1080.png", &width, &height, &channel,4);
     //   unsigned char* output_pixels = (unsigned char*)malloc(sizeof(unsigned char) * out_w * out_h * 4);

        if ((lut_width == 512 && lut_height == 512) || (lut_width == 64 && lut_height == 64)) {
            auto result = filters_.find(action_id);
           // auto r = prefilters_.find(action_id);
       /*    if (r == prefilters_.end()) {
                Filter *prefilter = NULL;
                prefilter = new Filter(720, 1280);
                prefilter->SetStartTime(start_time);
                prefilter->SetEndTime(end_time);
                prefilter->SetIntensity(intensity);
                prefilter->SetPortrait( 257 , 257 );  
                prefilters_ = prefilter;
              
            }*/
            if (result == filters_.end()) {
                // 添加filter
                Filter *filter = NULL;
  //              if (strstr(lut_path,"chahui"))
                
                filter = new Filter(lut_buffer, 720, 1280,1);
                filter->SetStartTime(start_time);
                filter->SetEndTime(end_time);
                filter->SetIntensity(intensity);
                filter->SetPortrait( 257 , 257 );  
            
            //    filter->GetPortrait( inputImage ,width ,height, output_pixels ,512 ,512);
             //   stbir_resize_uint8( outbuffer , width , height , 0, pMask, 512, 512, 0, 4);
          //      stbi_write_png("/sdcard/data/output.jpg", 257, 257, 4, output_pixels, 4 * 257);
                //            filter->SetPosition(face_keypoints);
              
                filters_.insert(std::pair<int, Filter*>(action_id,  filter));
              //  prefilters_.insert(std::pair<int, Filter*>(action_id,  prefilter));
            
            } else {
                // 更新filter
                Filter* filter = filters_[action_id];
                filter->SetStartTime(start_time);
                filter->SetEndTime(end_time);
                filter->SetIntensity(intensity);
    //            filter->SetPosition(face_keypoints);
                
                filter->UpdateLut(lut_buffer, 720, 1280);
            }
        }
        stbi_image_free(lut_buffer);
       // stbi_image_free( inputImage );
        //free(output_pixels);
    }
    cJSON_Delete(json);
}


void ImageProcess::OnDeleteFilter(int action_id) {
    auto result = filters_.find(action_id);
    if (result != filters_.end()) {
        Filter* filter = filters_[action_id];
        delete filter;
        filters_.erase(action_id);
    }
}

}  // namespace trinity
