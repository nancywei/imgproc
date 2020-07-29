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
#define FACE
namespace trinity {

  // beauty 磨皮 fragment shader
  static const char* BEAUTY_FRAGMENT_SHADER =
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
        "void main() {                                                                          \n"
        "vec4 sourceColor = texture2D(inputImageTexture, textureCoordinate);\n"
        "float fstep = 0.0015;\n"
        "vec2 tex1 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y);\n"
        "vec4 p1 = texture2D(inputImageTexture, tex1);\n"
        "vec2 tex2 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y);\n"
        "vec4 p2 = texture2D(inputImageTexture, tex2);\n"
        "vec2 tex3 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y);\n"
        "vec4 p3 = texture2D(inputImageTexture, tex3);\n"
        "vec2 tex4 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y);\n"
        "vec4 p4 = texture2D(inputImageTexture, tex4);\n"
        "vec4 line0 = (p2 + 4.0 * p1 + 6.0 * sourceColor + 4.0 * p3 + p4)/16.0;                            \n"

       "vec2 tex0 = vec2(textureCoordinate.x ,textureCoordinate.y + fstep);\n"
       "vec4 p0 = texture2D(inputImageTexture, tex0);\n"
       "vec2 tex5 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y + fstep);\n"
       "vec4 p5 = texture2D(inputImageTexture, tex5);\n"
       "vec2 tex6 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y + fstep);\n"
       "vec4 p6 = texture2D(inputImageTexture, tex6);\n"
       "vec2 tex7 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y + fstep);\n"
       "vec4 p7 = texture2D(inputImageTexture, tex7);\n"
       "vec2 tex8 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y + fstep);\n"
       "vec4 p8 = texture2D(inputImageTexture, tex8);\n"
       "vec4 line1 = (p6 + 4.0 * p5 + 6.0 * p0 + 4.0 * p7 + p8)/16.0;                            \n"

       "vec2 te0 = vec2(textureCoordinate.x ,textureCoordinate.y + 2.0 * fstep);\n"
       "vec4 pl0 = texture2D(inputImageTexture, te0);\n"
       "vec2 te5 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y + 2.0 * fstep);\n"
       "vec4 pl5 = texture2D(inputImageTexture, te5);\n"
       "vec2 te6 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y + 2.0 * fstep);\n"
       "vec4 pl6 = texture2D(inputImageTexture, te6);\n"
       "vec2 te7 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y + 2.0 * fstep);\n"
       "vec4 pl7 = texture2D(inputImageTexture, te7);\n"
       "vec2 te8 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y + 2.0 * fstep);\n"
       "vec4 pl8 = texture2D(inputImageTexture, te8);\n"
       "vec4 line2 = (pl6 + 4.0 * pl5 + 6.0 * pl0 + 4.0 * pl7 + pl8)/16.0;                            \n"

       "vec2 t0 = vec2(textureCoordinate.x ,textureCoordinate.y -  fstep);\n"
       "vec4 pr0 = texture2D(inputImageTexture, t0);\n"
       "vec2 t5 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y - fstep);\n"
       "vec4 pr5 = texture2D(inputImageTexture, t5);\n"
       "vec2 t6 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y -  fstep);\n"
       "vec4 pr6 = texture2D(inputImageTexture, t6);\n"
       "vec2 t7 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y - fstep);\n"
       "vec4 pr7 = texture2D(inputImageTexture, t7);\n"
       "vec2 t8 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y - fstep);\n"
       "vec4 pr8 = texture2D(inputImageTexture, t8);\n"
       "vec4 line3 = (pr6 + 4.0 * pr5 + 6.0 * pr0 + 4.0 * pr7 + pr8)/16.0;                            \n"

       "vec2 tx0 = vec2(textureCoordinate.x ,textureCoordinate.y - 2.0 * fstep);\n"
       "vec4 px0 = texture2D(inputImageTexture, tx0);\n"
       "vec2 tx5 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y - 2.0 * fstep);\n"
       "vec4 px5 = texture2D(inputImageTexture, tx5);\n"
       "vec2 tx6 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y - 2.0 * fstep);\n"
       "vec4 px6 = texture2D(inputImageTexture, tx6);\n"
       "vec2 tx7 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y - 2.0 * fstep);\n"
       "vec4 px7 = texture2D(inputImageTexture, tx7);\n"
       "vec2 tx8 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y - 2.0 * fstep);\n"
       "vec4 px8 = texture2D(inputImageTexture, tx8 );\n"
       "vec4 line4 = (px6 + 4.0 * px5 + 6.0 * px0 + 4.0 * px7 + px8)/16.0;                            \n"
       " if (sourceColor.r > 95.0/255.0 && sourceColor.g > 40.0/255.0 && sourceColor.b > 20.0/255.0 && sourceColor.r > sourceColor.b && sourceColor.r > sourceColor.g && abs(sourceColor.r - sourceColor.g ) > 15.0/255.0 ) {    \n"

       "vec4 p = ( line4 + 4.0 * line3 + 6.0 * line0 + 4.0 * line1 + line2)/15.0;\n"
    //    "vec4 p = line3 ;\n"
        "gl_FragColor = p;                                                                       \n"
        "}\n"
        "else  gl_FragColor = sourceColor;                          \n"
        "}                                                                                      \n";

  // 默认OES fragment shader
  static const char* RESET_FRAGMENT_SHADER =
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
        "void main() {                                                                          \n"
        "  gl_FragColor = texture2D(inputImageTexture, textureCoordinate);                          \n"
        "}                                                                                      \n";

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
            "uniform float ex;                                                                    \n"
            "uniform float ey;                                                                    \n"
            "uniform float Rex;                                                                    \n"
            "uniform float Rey;                                                                    \n"
            "uniform float ex1;                                                                    \n"
            "uniform float ey1;                                                                    \n"
            "uniform float Rex1;                                                                    \n"
            "uniform float Rey1;                                                                    \n"
            "uniform float strength;                                                                    \n"
            "uniform float ratio;                                                                    \n"
            "void main () {                                                                              \n"
            "vec2 textureCoordinateToUse = textureCoordinate;                                            \n"
            "float aspectRatio = ratio;                                                                    \n"
            "float delta = strength;   "
            "vec2 originPoint = vec2( ex, ey );                                                \n"
            "vec2 targetPoint = vec2( Rex , Rey );                                                \n"

            "float radius = distance(vec2(targetPoint.x, targetPoint. y / aspectRatio),vec2(originPoint.x, originPoint.y / aspectRatio));\n"
          //  "radius = radius * 5.;                                                                       \n"
            "float weight = distance(vec2(textureCoordinate.x, textureCoordinate.y / aspectRatio), vec2(originPoint.x, originPoint.y / aspectRatio)) / radius;\n"
            "weight = clamp(weight, 0.0, 1.0);                                                           \n"
            "weight = 1.0 - (1.0 - weight * weight) * delta ;                                          \n"

            "vec2 originPoint1 = vec2( ex1, ey1 );                                                \n"
            "vec2 targetPoint1 = vec2( Rex1 , Rey1 );                                                \n"
            "float radius1 = distance(vec2(targetPoint1.x, targetPoint1. y / aspectRatio),vec2(originPoint1.x, originPoint1.y / aspectRatio));\n"
            //  "radius = radius * 5.;                                                                       \n"
            "float weight1 = distance(vec2(textureCoordinate.x, textureCoordinate.y / aspectRatio), vec2(originPoint1.x, originPoint1.y / aspectRatio)) / radius1;\n"
            "weight1 = clamp(weight1, 0.0, 1.0);                                                           \n"
            "weight1 = 1.0 - (1.0 - weight1 * weight1) * delta ;                                          \n"

            "textureCoordinateToUse = originPoint + (textureCoordinate - originPoint) * weight ;          \n"
            "textureCoordinateToUse = originPoint1 + (textureCoordinateToUse - originPoint1) * weight1;\n"
            "gl_FragColor=texture2D(inputImageTexture, textureCoordinateToUse);                          \n"
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
            "uniform float strength;                                                                    \n"
            "uniform float intensity;                                                                    \n"
            "uniform float Rx1;                                                                          \n"
            "uniform float Ry1;                                                                          \n"
            "uniform float plx;                                                                          \n"
            "uniform float ply;                                                                          \n"
            "uniform float prx;                                                                          \n"
            "uniform float pry;                                                                          \n"
            "void main () {                                                                              \n"
  //          "vec2 textureCoordinateToUse1 = textureCoordinate;                                            \n"
            "vec2 textureCoordinateToUse = textureCoordinate;                                            \n"
            "vec2 textureCoordinateToUse1 = textureCoordinateToUse; \n"
            "float ar = ratio;                                                     \n"
            "float st = strength               ;                                                   \n"
            "if(plx > 0. && prx > 0.){                                                                   \n"
            "vec2 originPoint = vec2( plx, ply );                                                        \n"
            "vec2 RPoint = vec2( Rx, Ry );                                                        \n"
            "float tx = plx + (Rx - plx)/st;      \n"
            "float ty = ply + (Ry - ply)/st;      \n"
            "vec2 targetPoint = vec2( tx, ty );                                                        \n"
            "float R = distance(vec2(originPoint.x,originPoint.y/ar),vec2(RPoint.x,RPoint.y/ar));                                                     \n"
            "float d = distance(vec2(textureCoordinate.x, textureCoordinate.y/ar), vec2(originPoint.x, originPoint.y/ar));\n"
            "float weight = 0. ;\n"
            "vec2 originPoint1 = vec2( prx, pry );                                                        \n"
            "vec2 RPoint1 = vec2( Rx1, Ry1 );                                                        \n"
            "float tx1 = prx + (Rx1 - prx)/st;      \n"
            "float ty1 = pry + (Ry1 - pry)/st;      \n"
            "vec2 targetPoint1 = vec2( tx1, ty1 );                                                        \n"
            "float R1 = distance(vec2(originPoint1.x,originPoint1.y/ar),vec2(RPoint1.x,RPoint1.y/ar));                                                     \n"
            "float d1 = distance(vec2(textureCoordinateToUse.x, textureCoordinateToUse.y/ar), vec2(originPoint1.x, originPoint1.y/ar));\n"

            "if( d < R || d1 < R1){\n"
            "float dv = distance(vec2(originPoint.x,originPoint.y/ar), vec2(targetPoint.x,targetPoint.y/ar));  \n"
 //           "float enhance = sqrt(d); \n"
  //          "enhance = clamp(enhance, 1.0, 5.0);                                                           \n"
            "weight = (R - d)/abs(R - d + dv * strength/2.0) ;                                                             \n"
            "weight = clamp(weight, 0.0, 1.0);                                                           \n"
            "weight = weight * weight;                                                                   \n"
            "vec2 Vxy =  originPoint - targetPoint;                             \n"
            "float d1v = distance(vec2(originPoint1.x,originPoint1.y/ar), vec2(targetPoint1.x,targetPoint1.y/ar));  \n"
            //         "float enhance = sqrt(d)/10.0; \n"
            "float weight1 = (R1 - d1)/abs(R1 - d1 + d1v * strength/3.0) ;                                                             \n"
            "weight1 = clamp(weight1, 0.0, 1.0);                                                           \n"
            "weight1 = weight1 * weight1;                                                                   \n"
            "vec2 V1xy =  originPoint1 - targetPoint1;                             \n"

            "textureCoordinateToUse = textureCoordinate + Vxy * weight + V1xy * weight1;          \n"

            "} \n"


            "} \n"
            "gl_FragColor=texture2D(inputImageTexture, textureCoordinateToUse);                          \n"
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

    static const char* FACECOLOR_FRAGMENT_SHADER =
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
        "uniform float strength;                                                                    \n"
        "uniform float left;                                                                    \n"
        "uniform float right;                                                                    \n"
        "uniform float bottom;                                                                    \n"
        "uniform float top;                                                                    \n"
 
        "void main () {                                                                              \n"
        " vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);                       \n"
 //       " if (textureCoordinate.y < top && textureCoordinate.x > left && textureCoordinate.x < right ) {    \n"
        " if (textureCoordinate.y < 1.0 && textureColor.r > 95.0/255.0 && textureColor.g > 40.0/255.0 && textureColor.b > 20.0/255.0 && textureColor.r > textureColor.b && textureColor.r > textureColor.g && abs(textureColor.r - textureColor.g ) > 15.0/255.0 ) {    \n"
  //      " if( max(textureColor.r,textureColor.g,textureColor.b) - min(textureColor.r,textureColor.g,textureColor.b) >15.0/255.0) { \n"
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
 //       " }                                                                                          \n"
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
    NormalFilter(int width, int height ,const char *fragment ) : FrameBuffer(width, height, DEFAULT_VERTEX_SHADER,fragment) {}
    
    NormalFilter(uint8_t* filter_buffer, int width, int height,const char *fragment ) : FrameBuffer(width, height, DEFAULT_VERTEX_SHADER, fragment) {
        intensity_ = 1.0f;
        filter_texture_id_ = 0;
        position_ = NULL;
 
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

    void SetIntensity(float intensity ) {
        intensity_ = intensity;
    }
 
 
#ifdef FACE
    void SetStrength(float strength ) {
        strength_ = strength;
    }

    void SetAspectratio(float aspectratio ) {
        aspectratio_ = aspectratio;
    }
  
    void SetFaceedge(float* position ) {
        left_ = * position;
        right_ = * (position + 1);
        bottom_ = * (position + 2);
        top_ = * (position + 3);
    }
   
    void SetPosition(float* position ) {
        position_ = position;
        plx_ = * position;
        ply_ = * (position + 1);
        Rx_ = * (position + 2);
        Ry_ = * (position + 3);
        prx_ = * (position + 4);
        pry_ = * (position + 5);
        Rx1_ = * (position + 6);
        Ry1_ = * (position + 7);
        ex_ = * (position + 8);
        ey_ = * (position + 9);
        Rex_ = * (position + 10);
        Rey_ = * (position + 11);
        ex1_ = * (position + 12);
        ey1_ = * (position + 13);
        Rex1_ = * (position + 14);
        Rey1_ = * (position + 15);

    }
#endif

    ~NormalFilter() {
        if (filter_texture_id_ != 0) {
            glDeleteTextures(1, &filter_texture_id_);
            filter_texture_id_ = 0;
        }
    }

 private:
    GLuint filter_texture_id_;
    float intensity_;
    float strength_ = 10.0;
    float *position_;
    float plx_ = 0.;
    float ply_ = 0.;
    float Rx_ = 0.;
    float Ry_ = 0.;
    float prx_ = 0.;
    float pry_ = 0.;
    float Rx1_ = 0.;
    float Ry1_ = 0.;
    float ex_ = 0.;
    float ey_ = 0.;
    float Rex_ = 0.;
    float Rey_ = 0.;
    float ex1_ = 0.;
    float ey1_ = 0.;
    float Rex1_ = 0.;
    float Rey1_ = 0.;
    float aspectratio_ = 1.0;
    // for face edge position
    float left_ = 0.;
    float right_ = 0.;
    float bottom_ = 0.;
    float top_ = 0.;
    

    protected:
    virtual void RunOnDrawTasks() {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, filter_texture_id_);
        SetInt("inputImageTextureLookup", 1);
        SetFloat("intensity", intensity_);
        SetFloat("strength", strength_);
        SetFloat("ratio", aspectratio_);
        

#ifdef FACE
        if (position_){
// slim face ralative
            SetFloat("plx", plx_);
            SetFloat("ply", ply_);
            SetFloat("Rx", Rx_);
            SetFloat("Ry", Ry_);
            SetFloat("prx", prx_);
            SetFloat("pry", pry_);
            SetFloat("Rx1", Rx1_);
            SetFloat("Ry1", Ry1_);
// enlarge eye ralative 
            SetFloat("ex", ex_);
            SetFloat("ey", ey_);
            SetFloat("Rex", Rex_);
            SetFloat("Rey", Rey_);

            SetFloat("ex1", ex1_);
            SetFloat("ey1", ey1_);
            SetFloat("Rex1", Rex1_);
            SetFloat("Rey1", Rey1_);
// beutify face color ralative
            SetFloat("left", left_);
            SetFloat("right", right_);
            SetFloat("bottom", bottom_);
            SetFloat("top", top_);
        
        }
#endif

    }
};

class Filter : public NormalFilter {
 public:
    Filter(int width, int height) : NormalFilter(width, height, SLIM_FRAGMENT_SHADER) {}
    Filter(int width, int height,int type) : NormalFilter(width, height, ENLARGE_FRAGMENT_SHADER) {}
    Filter(int width, int height,int type,int type1) : NormalFilter(width, height, RESET_FRAGMENT_SHADER) {}
    Filter(int width, int height,int type,int type1,int type2) : NormalFilter(width, height, BEAUTY_FRAGMENT_SHADER) {}
  
    Filter(uint8_t* filter_buffer, int width, int height, int type) : NormalFilter( filter_buffer, width, height, FACECOLOR_FRAGMENT_SHADER ) {}

    Filter(uint8_t* filter_buffer, int width, int height) : NormalFilter( filter_buffer, width, height, FILTER_FRAGMENT_SHADER ) {}

};


}  // namespace trinity

#endif  // TRINITY_FILTER_H
