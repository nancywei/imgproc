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
//#include <utility>

#if __ANDROID__
//#include "android_xlog.h"
#elif __APPLE__
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#define LOGE(format, ...) fprintf(stdout, format, __VA_ARGS__) // NOLINT
#define LOGI(format, ...) fprintf(stdout, format, __VA_ARGS__) // NOLINT
#endif

namespace trinity {

    ImageProcess::ImageProcess()
            : action_id_(-1) {}

    ImageProcess::~ImageProcess() {
        OnDeleteAllFilters();

    }

    GLuint ImageProcess::Process(GLuint texture_id, int width, int height) {
        GLuint texture = texture_id;
        for (auto &filter : filters_) {
            Filter *f = filter.second;
            GLuint process_texture = f->OnDrawFrame(texture);
            texture = process_texture;
        }
        return texture;
    }

    GLuint ImageProcess::Process(GLuint texture_id, int64_t current_time, int width, int height,
                                 int input_color_type, int output_color_type) {
        return OnProcess(texture_id, current_time, width, height);
    }

    int ImageProcess::Process(uint8_t *frame, uint64_t current_time, int width, int height,
                              int input_color_type, int output_color_type) {
        return OnProcess(0, current_time, width, height);
    }

    GLuint ImageProcess::OnProcess(GLuint texture_id, int64_t current_time, int width, int height) {
        GLuint texture = texture_id;
        // 执行滤镜操作
        for (auto &filter : filters_) {
            Filter *f = filter.second;
            if (has_face) {
                f->SetPosition(face_keypoints);
                //           f->SetFaceedge(face_edge);
            }
            float aspectratio = float(width) / float(height);
            f->SetAspectratio(aspectratio);
            GLuint process_texture = f->OnDrawFrame(texture, current_time);
            texture = process_texture;
        }

        return texture;
    }

    int ImageProcess::OnBeautyFilter() {

        int action_id = action_id_ + 1;
        Filter *filter = NULL;
        float intensity = 1.0;
        float strength = 1.0;
        filter = new Filter(720, 1280, 1, 1, 1);  //beauty
        filter->SetStartTime(0);
        filter->SetEndTime(INT32_MAX);
        filter->SetIntensity(intensity);
        filter->SetStrength(strength);
        filters_.insert(std::pair<int, Filter *>(action_id, filter));
        action_id_++;
        return action_id;

    }

    void
    ImageProcess::OnFilter(const char *config_path, int action_id, int start_time, int end_time) {
        Filter *filter = NULL;
        float intensity = 1.0;
        float strength = 1.0;
        filter = new Filter(720, 1280, 1, 1, 1);  //beauty
        filter->SetStartTime(start_time);
        filter->SetEndTime(end_time);
        filter->SetIntensity(intensity);
        filter->SetStrength(strength);
        filters_.insert(std::pair<int, Filter *>(action_id, filter));
    }

    void ImageProcess::OnDeleteFilter(int action_id) {
        auto result = filters_.find(action_id);
        if (result != filters_.end()) {
            Filter *filter = filters_[action_id];
            delete filter;
            filters_.erase(action_id);
        }
    }

    void ImageProcess::OnDeleteAllFilters() {
        for (auto &filter : filters_) {
            Filter *f = filter.second;
            delete f;
            filters_.erase(filter.first);
        }
    }

}// namespace trinity
