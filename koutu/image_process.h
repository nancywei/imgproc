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

#ifndef TRINITY_IMAGE_PROCESS_H
#define TRINITY_IMAGE_PROCESS_H

#include <cstdint>
#include <vector>
#include <map>
#include "frame_buffer.h"
#include "filter.h"
#include "image_buffer.h"
#include "opengl_observer.h"
#include "process_buffer.h"
#include "effect.h"

extern "C" {
#include "cJSON.h"
};

#define NO_ACTION -1
#define HORIZONTAL 0
#define VERTICAL 1

namespace trinity {

class ImageProcess {
 public:
    ImageProcess();
    ~ImageProcess();

    int buffer_length_ = 111 * 2 * sizeof(float);
    float* face_keypoints = new float[buffer_length_];

    bool has_face = false;
    GLuint Process(GLuint texture_id, int64_t current_time,
            int width, int height,
            int input_color_type, int output_color_type);

    int Process(uint8_t* frame, uint64_t current_time,
            int width, int height,
            int input_color_type, int output_color_type);

    void OnAction(char* config_path, int action_id, FaceDetection* face_detection = nullptr);
    void OnUpdateActionTime(int start_time, int end_time, int action_id);
    void OnUpdateEffectParam(int action_id, const char* effect_name, const char* param_name, float value);
    void OnUpdateEffectParam(int action_id, const char* effect_name, const char* param_name, float* value, int length);
    void RemoveAction(int action_id);
    void ClearAction();
    void OnAIFilter(const char* config_path, int action_id, int start_time = 0, int end_time = INT32_MAX,int width = 0,int height = 0);
    
    void OnFilter(const char* config_path, int action_id, int start_time = 0, int end_time = INT32_MAX);
    void OnDeleteFilter(int action_id);

 private:
    /**
     * 解析json配置
     * @param config
     * @param action_id
     */
    void ParseConfig(char* config_path, int action_id = -1);
    
    int ReadFile(char* path, char** buffer);

    /**
     * 执行特效处理
     * @param texture_id
     * @param current_time
     * @param width
     * @param height
     * @return
     */
    GLuint OnProcess(GLuint texture_id, int64_t current_time, int width, int height);

 private:
    std::map<int, Filter*> filters_;
    std::map<int, Effect*> effects_;
    int action_id_;
    Filter*  prefilters_;
    bool frame_status = false;
    Inference_engine *infer = new Inference_engine();
    unsigned char* pmask_ = new unsigned char[sizeof (unsigned char) * 512 * 512 * 4];
    unsigned char* pmask512_ = new unsigned char[sizeof (unsigned char) * 512 * 512 * 4];

    unsigned char* pResult_ = new unsigned char[sizeof (unsigned char) * 512 * 512 * 4];

    unsigned char* pbuffer_ = new unsigned char[sizeof (unsigned char) * 1280 * 720 * 4];

    //Filter *prefilter_ = NULL;
    //Filter * prefilter_ = new Filter(720, 1280);
               
           
};

}  // namespace trinity

#endif  // TRINITY_IMAGE_PROCESS_H
