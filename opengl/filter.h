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
 *
 */

//
// Created by wlanjie on 2019-06-11.
//

#ifndef TRINITY_FILTER_H
#define TRINITY_FILTER_H

#include "frame_buffer.h"
#include "gl.h"

#include <stdio.h>
#include <ImageProcess.hpp>
#define MNN_OPEN_TIME_TRACE
#include <algorithm>
#include <fstream>
#include <functional>
#include <memory>
#include <sstream>
#include <vector>
#include <expr/Expr.hpp>
#include <expr/ExprCreator.hpp>
#include <AutoTime.hpp>

namespace trinity {

    static const char* MASK_FRAGMENT_SHADER =
            "#ifdef GL_ES                                                                                \n"
            "precision highp float;                                                                      \n"
            "varying highp vec2 textureCoordinate;                                                       \n"
            "varying highp vec2 textureCoordinate2;                                                      \n"
            "#else                                                                                       \n"
            "varying vec2 textureCoordinate;                                                             \n"
            "varying vec2 textureCoordinate2;                                                            \n"
            "#endif                                                                                      \n"
            "uniform sampler2D inputImageTexture;                                                        \n"
            "uniform sampler2D inputImageTextureLookup;                                                  \n"
            "void main () {                                                                              \n"
            "vec4 textureColor2 = texture2D(inputImageTextureLookup, textureCoordinate);                 \n"
            "vec4 textureColor1 = texture2D(inputImageTexture, textureCoordinate);                       \n"
            "float mask = textureColor2.r;                                                               \n"
            "if(mask == 0.0) gl_FragColor = textureColor2;                                                     \n"
            "else gl_FragColor = textureColor1;                                                          \n"
            "}\n";
    
// ratio is camera_height/camera_with; tlx ,tly is target keypoint of right face keypoint 8;
   static const char* SLIM_FRAGMENT_SHADER =
            "#ifdef GL_ES                                                                                \n"
            "precision highp float;                                                                      \n"
            "varying highp vec2 textureCoordinate;                                                       \n"
            "varying highp vec2 textureCoordinate2;                                                      \n"
            "#else                                                                                       \n"
            "varying vec2 textureCoordinate;                                                             \n"
            "varying vec2 textureCoordinate2;                                                            \n"
            "#endif                                                                                      \n"
            "uniform sampler2D inputImageTexture;                                                        \n"
            "uniform sampler2D inputImageTextureLookup;                                                  \n"
            "uniform float Rx;                                                                    \n"
            "uniform float Ry;                                                                          \n"
            "uniform float ratio;                                                                          \n"
            "uniform float tlx;                                                                          \n"
            "uniform float tly;                                                                          \n"
            "uniform float plx;                                                                          \n"
            "uniform float ply;                                                                          \n"
            "uniform float rlx;                                                                          \n"
            "uniform float rly;                                                                          \n"
            "void main () {                                                                              \n"
            "vec2 textureCoordinateToUse = textureCoordinate;                                            \n"
            "if(plx > 0.){                                                                               \n"
            "vec2 originPoint = vec2( plx, ply );                                                        \n"
            "vec2 targetPoint = vec2( plx + 0.05, ply + 0.05 );                                                        \n"
            "vec2 RPoint = vec2( plx + 0.1, ply + 0.1 );                                                        \n"
            //         "vec2 targetPoint = vec2( plx + (Rx - plx)/5.0, ply + (Ry - ply)/5.0 );                      \n"
   //         "vec2 targetPoint = vec2(tlx , tly );\n"
   //         "vec2 RPoint = vec2( rlx , rly );                                                               \n"
            "float dv = distance(vec2(originPoint.x,originPoint.y), vec2(targetPoint.x,targetPoint.y));  \n"
            "float R = distance(vec2(originPoint.x,originPoint.y),vec2(RPoint.x,RPoint.y));                                                     \n"
            "float d = distance(vec2(textureCoordinate.x, textureCoordinate.y), vec2(originPoint.x, originPoint.y));\n"
            "if( d < R ){\n"
            "float weight = (R - d)/(R - d + dv) ;                                                             \n"
            "weight = weight * weight;                                                                   \n"
            "weight = clamp(weight, 0.0, 1.0);                                                           \n"
            "vec2 Vxy =  originPoint - targetPoint;                             \n"
            "textureCoordinateToUse = textureCoordinate + Vxy * weight;          \n"
            
            "}                                                                                           \n"
            "} \n"
            "gl_FragColor=texture2D(inputImageTexture, textureCoordinateToUse);                          \n"
            "}\n";

    static const char* ENLARGE_FRAGMENT_SHADER =
            "#ifdef GL_ES                                                                                \n"
            "precision highp float;                                                                      \n"
            "varying highp vec2 textureCoordinate;                                                       \n"
            "varying highp vec2 textureCoordinate2;                                                      \n"
            "#else                                                                                       \n"
            "varying vec2 textureCoordinate;                                                             \n"
            "varying vec2 textureCoordinate2;                                                            \n"
            "#endif                                                                                      \n"
            "uniform sampler2D inputImageTexture;                                                        \n"
            "uniform sampler2D inputImageTextureLookup;                                                  \n"
            "uniform float intensity;                                                                    \n"
            "uniform float plx;                                                                          \n"
            "uniform float ply;                                                                          \n"
            "uniform float prx;                                                                          \n"
            "uniform float pry;                                                                          \n"
            "void main () {                                                                              \n"
            "vec2 textureCoordinateToUse = textureCoordinate;                                            \n"
            "float aspectRatio = 1.0;                                                                    \n"
            "float delta = 0.5;   "
            "if(plx > 0.){                                                                               \n"
            "vec2 originPoint = vec2( plx, ply );                                                        \n"
            "vec2 targetPoint = vec2( plx + 0.1, ply + 0.1 );                                            \n"
            "float radius = distance(vec2(targetPoint.x, targetPoint. y / aspectRatio),vec2(originPoint.x, originPoint.y / aspectRatio));\n"
          //  "radius = radius * 5.;                                                                       \n"
            "float weight = distance(vec2(textureCoordinate.x, textureCoordinate.y / aspectRatio), vec2(originPoint.x, originPoint.y / aspectRatio)) / radius;\n"
            "weight = 1.0 - (1.0 - weight * weight) * delta ;                                          \n"
            "weight = clamp(weight, 0.0, 1.0);                                                           \n"
            "textureCoordinateToUse = originPoint + (textureCoordinate - originPoint) * weight;          \n"
            "}                                                                                           \n"
            "gl_FragColor=texture2D(inputImageTexture, textureCoordinateToUse);                          \n"
            "}\n";

    static const char* BEUTY_VERTEX_SHADER =
        "#ifdef GL_ES                                                                                \n"
        "precision highp float;                                                                      \n"
        "#endif                                                                                      \n"
        "attribute vec4 position;                                                                    \n"
        "attribute vec2 inputTextureCoordinate;                                                      \n"
        "varying vec2 textureCoordinate;                                                             \n"
        "varying vec2 textureCoordinate2;                                                            \n"
        "void main() {                                                                               \n"
        "    gl_Position = position;                                                                 \n"
        "    textureCoordinate = inputTextureCoordinate.xy;                                          \n"
        "    textureCoordinate2 = inputTextureCoordinate.xy * 0.5 + 0.5;                             \n"
        "}                                                                                           \n";

    static const char* BILTER_FRAGMENT_SHADER =
        "#ifdef GL_ES                                                                                \n"
        "precision highp float;                                                                      \n"
        "varying highp vec2 textureCoordinate;                                                       \n"
        "varying highp vec2 textureCoordinate2;                                                      \n"
        "#else                                                                                       \n"
        "varying vec2 textureCoordinate;                                                             \n"
        "varying vec2 textureCoordinate2;                                                            \n"
        "#endif                                                                                      \n"
        "uniform sampler2D inputImageTexture;                                                        \n"
        "uniform sampler2D inputImageTextureLookup;                                                  \n"
        "uniform float intensity;                                                                    \n"
        "void main () {                                                                              \n"
        " vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);                       \n"
        " if (textureCoordinate.y < 0.5) {                                                           \n"
        "     float yColor = textureColor.b * 63.0;                                                  \n"
        "     vec2 quad1;                                                                            \n"
        "     quad1.y = floor(floor(yColor) / 8.0);                                                  \n"
        "     quad1.x = floor(yColor) - (quad1.y * 8.0);                                             \n"
        "     vec2 quad2;                                                                            \n"
        "     quad2.y = floor(ceil(yColor) / 8.0);                                                   \n"
        "     quad2.x = ceil(yColor) - (quad2.y * 8.0);                                              \n"
        "     vec2 texPos1;                                                                          \n"
        "     texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);    \n"
        "     texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);    \n"
        "     vec2 texPos2;                                                                          \n"
        "     texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);    \n"
        "     texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);    \n"
        "     vec4 newColor1;                                                                        \n"
        "     vec4 newColor2;                                                                        \n"
        "     newColor1 = texture2D(inputImageTextureLookup, texPos1);                               \n"
        "     newColor2 = texture2D(inputImageTextureLookup, texPos2);                               \n"
        "     vec4 newColor = mix(newColor1, newColor2, fract(yColor));                              \n"
        "     gl_FragColor = mix(textureColor, vec4(newColor.rgb, textureColor.a), intensity);       \n"
        " } else {                                                                                   \n"
        "     gl_FragColor = textureColor;                                                           \n"
        " }                                                                                          \n"
        "}\n";

    static const char* FILTER_FRAGMENT_SHADER =
            "#ifdef GL_ES                                                                                \n"
            "precision highp float;                                                                      \n"
            "varying highp vec2 textureCoordinate;                                                       \n"
            "varying highp vec2 textureCoordinate2;                                                      \n"
            "#else                                                                                       \n"
            "varying vec2 textureCoordinate;                                                             \n"
            "varying vec2 textureCoordinate2;                                                            \n"
            "#endif                                                                                      \n"
            "uniform sampler2D inputImageTexture;                                                        \n"
            "uniform sampler2D inputImageTextureLookup;                                                  \n"
            "uniform float intensity;                                                                    \n"
            "void main () {                                                                              \n"
            " vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);                       \n"
            " if (textureCoordinate.y < 1.0) {                                                           \n"
            "     float yColor = textureColor.b * 63.0;                                                  \n"
            "     vec2 quad1;                                                                            \n"
            "     quad1.y = floor(floor(yColor) / 8.0);                                                  \n"
            "     quad1.x = floor(yColor) - (quad1.y * 8.0);                                             \n"
            "     vec2 quad2;                                                                            \n"
            "     quad2.y = floor(ceil(yColor) / 8.0);                                                   \n"
            "     quad2.x = ceil(yColor) - (quad2.y * 8.0);                                              \n"
            "     vec2 texPos1;                                                                          \n"
            "     texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);    \n"
            "     texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);    \n"
            "     vec2 texPos2;                                                                          \n"
            "     texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);    \n"
            "     texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);    \n"
            "     vec4 newColor1;                                                                        \n"
            "     vec4 newColor2;                                                                        \n"
            "     newColor1 = texture2D(inputImageTextureLookup, texPos1);                               \n"
            "     newColor2 = texture2D(inputImageTextureLookup, texPos2);                               \n"
            "     vec4 newColor = mix(newColor1, newColor2, fract(yColor));                              \n"
            "     gl_FragColor = mix(textureColor, vec4(newColor.rgb, textureColor.a), intensity);       \n"
            " } else {                                                                                   \n"
            "     gl_FragColor = textureColor;                                                           \n"
            " }                                                                                          \n"
            "}\n";


    static const char* FILTER_VERTEX_SHADER =
        "#ifdef GL_ES                                                                                \n"
        "precision highp float;                                                                      \n"
        "#endif                                                                                      \n"
        "attribute vec4 position;                                                                    \n"
        "attribute vec2 inputTextureCoordinate;                                                      \n"
        "varying vec2 textureCoordinate;                                                             \n"
        "varying vec2 textureCoordinate2;                                                            \n"
        "void main() {                                                                               \n"
        "    gl_Position = position;                                                                 \n"
        "    textureCoordinate = inputTextureCoordinate.xy;                                          \n"
        "    textureCoordinate2 = inputTextureCoordinate.xy * 0.5 + 0.5;                             \n"
        "}                                                                                           \n";


class NormalFilter : public FrameBuffer {
 public:
        NormalFilter(uint8_t* filter_buffer, int width, int height ,const char *fragment ) : FrameBuffer(width, height, DEFAULT_VERTEX_SHADER,fragment) {
        intensity_ = 1.0f;
        position_ = NULL;
        
        filter_texture_id_ = 0;
        glGenTextures(1, &filter_texture_id_);
        glBindTexture(GL_TEXTURE_2D, filter_texture_id_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, filter_buffer);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void UpdateLut(uint8_t* lut, int width, int height) {
        glBindTexture(GL_TEXTURE_2D, filter_texture_id_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, lut);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void SetIntensity(float intensity = 1.0f) {
        intensity_ = intensity;
    }

    void SetPosition(float* position ) {
        position_ = position;
        plx_ = * position;
        ply_ = * (position + 1);
        Rx_ = * (position + 2);
        Ry_ = * (position + 3);

    }

    ~NormalFilter() {
        if (filter_texture_id_ != 0) {
            glDeleteTextures(1, &filter_texture_id_);
            filter_texture_id_ = 0;
        }
    }

 private:
    GLuint filter_texture_id_;
    float intensity_;
    float *position_;
    float plx_ = 0.;
    float ply_ = 0.;
    float Rx_ = 0.;
    float Ry_ = 0.;

 protected:
    virtual void RunOnDrawTasks() {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, filter_texture_id_);
        SetInt("inputImageTextureLookup", 1);
        SetFloat("intensity", intensity_);
        if (position_){
            SetFloat("plx", plx_);
            SetFloat("ply", ply_);
            SetFloat("Rx", Rx_);
            SetFloat("Ry", Ry_);

        }
    }
};


class Filter : public NormalFilter {
 public:
    Filter(uint8_t* filter_buffer,int width, int height, float* position) : NormalFilter( filter_buffer, width, height,ENLARGE_FRAGMENT_SHADER) {}

    Filter(uint8_t* filter_buffer, int width, int height, int ftype) : NormalFilter( filter_buffer, width, height, SLIM_FRAGMENT_SHADER) {}

    Filter(uint8_t* filter_buffer, int width, int height) : NormalFilter( filter_buffer, width, height, FILTER_FRAGMENT_SHADER ) {}

};

#if 0
class AFilter : public NormalFilter {
public:
    AFilter(uint8_t* filter_buffer, int width, int height) : NormalFilter( filter_buffer, width, height,FILTER_FRAGMENT_SHADER) {}

};

class BFilter : public NormalFilter {
public:
    BFilter(uint8_t* filter_buffer, int width, int height ) : NormalFilter( filter_buffer, width, height,BILTER_FRAGMENT_SHADER) {}

};


class selectFilter {
 public:
    selectFilter();
    static Filter* CreateFilter(uint8_t* filter_buffer, int width, int height ,int ftype) {
        switch (ftype) {
            case 0:
                return dynamic_cast <Filter *> (new BFilter(filter_buffer, width, height));
            default:
                return dynamic_cast <Filter *>  (new AFilter(filter_buffer, width, height));
        }
    }
};
#endif
/*
class EnLargeFilter : public NormalFilter {
 public:
    EnLargeFilter(uint8_t* filter_buffer, int width, int height) : NormalFilter( filter_buffer, width, height,ENLARGE_FRAGMENT_SHADER) {}
};*/


}  // namespace trinity

#endif  // TRINITY_FILTER_H
