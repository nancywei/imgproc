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
#include <MNN/ImageProcess.hpp>
#define MNN_OPEN_TIME_TRACE
#include <algorithm>
#include <fstream>
#include <functional>
#include <memory>
#include <sstream>
#include <vector>
#include <MNN/expr/Expr.hpp>
#include <MNN/expr/ExprCreator.hpp>
#include <MNN/AutoTime.hpp>
#include "net.h"
//#define STB_IMAGE_RESIZE_IMPLEMENTATION
//#include "stb_image_resize.h"

namespace trinity {

    static const char* ROTATE_FRAGMENT_SHADER = 
            "#ifdef GL_ES                                                                                \n"
            "precision highp float;                                                                      \n"
            "varying highp vec2 textureCoordinate;                                                       \n"
            "#else                                                                                       \n"
            "varying vec2 textureCoordinate;                                                             \n"
            "#endif                                                                                      \n"
            "uniform sampler2D inputImageTexture;                                                        \n"
            "void main () {                                                                              \n"
            "gl_FragColor = texture2D(inputImageTexture, vec2(textureCoordinate.x,1.0-textureCoordinate.y));  \n"
            "}\n";

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
            "uniform float enablemask;                                                                   \n"
            "vec4 newcolor(float fstep,sampler2D inputImageTexture,vec2 textureCoordinate){ \n"
            "float ystep = fstep; \n"
            "vec4 sourceColor = texture2D(inputImageTexture, textureCoordinate);                       \n"
            "vec2 tex1 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y);\n"
            "vec4 p1 = texture2D(inputImageTexture, tex1);\n"
            "vec2 tex2 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y);\n"
            "vec4 p2 = texture2D(inputImageTexture, tex2);\n"
            "vec2 tex3 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y);\n"
            "vec4 p3 = texture2D(inputImageTexture, tex3);\n"
            "vec2 tex4 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y);\n"
            "vec4 p4 = texture2D(inputImageTexture, tex4);\n"
            "vec4 line0 = (p2 +  p1 +  sourceColor +  p3 + p4)/5.0;                            \n"
            "vec2 tex0 = vec2(textureCoordinate.x ,textureCoordinate.y + ystep );\n"
            "vec4 p0 = texture2D(inputImageTexture, tex0);\n"
            "vec2 tex5 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y + ystep);\n"
            "vec4 p5 = texture2D(inputImageTexture, tex5);\n"
            "vec2 tex6 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y + ystep);\n"
            "vec4 p6 = texture2D(inputImageTexture, tex6);\n"
            "vec2 tex7 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y + ystep);\n"
            "vec4 p7 = texture2D(inputImageTexture, tex7);\n"
            "vec2 tex8 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y + ystep);\n"
            "vec4 p8 = texture2D(inputImageTexture, tex8);\n"
            "vec4 line1 = (p6 +  p5 +  p0 +  p7 + p8)/5.0;                            \n"

            "vec2 te0 = vec2(textureCoordinate.x ,textureCoordinate.y + 2.0 * ystep);\n"
            "vec4 pl0 = texture2D(inputImageTexture, te0);\n"
            "vec2 te5 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y + 2.0 * ystep);\n"
            "vec4 pl5 = texture2D(inputImageTexture, te5);\n"
            "vec2 te6 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y + 2.0 * ystep);\n"
            "vec4 pl6 = texture2D(inputImageTexture, te6);\n"
            "vec2 te7 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y + 2.0 * ystep);\n"
            "vec4 pl7 = texture2D(inputImageTexture, te7);\n"
            "vec2 te8 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y + 2.0 * ystep);\n"
            "vec4 pl8 = texture2D(inputImageTexture, te8);\n"
            "vec4 line2 = (pl6 +  pl5 +  pl0 + pl7 + pl8)/5.0;                            \n"

            "vec2 t0 = vec2(textureCoordinate.x ,textureCoordinate.y -  ystep);\n"
            "vec4 pr0 = texture2D(inputImageTexture, t0);\n"
            "vec2 t5 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y - ystep);\n"
            "vec4 pr5 = texture2D(inputImageTexture, t5);\n"
            "vec2 t6 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y -  ystep);\n"
            "vec4 pr6 = texture2D(inputImageTexture, t6);\n"
            "vec2 t7 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y - ystep);\n"
            "vec4 pr7 = texture2D(inputImageTexture, t7);\n"
            "vec2 t8 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y - ystep);\n"
            "vec4 pr8 = texture2D(inputImageTexture, t8);\n"
            "vec4 line3 = (pr6 +  pr5 +  pr0 +  pr7 + pr8)/5.0;                            \n"

            "vec2 tx0 = vec2(textureCoordinate.x ,textureCoordinate.y - 2.0 * ystep);\n"
            "vec4 px0 = texture2D(inputImageTexture, tx0);\n"
            "vec2 tx5 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y - 2.0 * ystep);\n"
            "vec4 px5 = texture2D(inputImageTexture, tx5);\n"
            "vec2 tx6 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y - 2.0 * ystep);\n"
            "vec4 px6 = texture2D(inputImageTexture, tx6);\n"
            "vec2 tx7 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y - 2.0 * ystep);\n"
            "vec4 px7 = texture2D(inputImageTexture, tx7);\n"
            "vec2 tx8 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y - 2.0 * ystep);\n"
            "vec4 px8 = texture2D(inputImageTexture, tx8 );\n"
            "vec4 line4 = (px6 + px5 +  px0 +  px7 + px8)/5.0;                            \n"

            "vec4 p = (line0 + line1 + line2 + line3 + line4 )/5.0; \n"

            " return p ;\n"
            "} \n"
            "void main () {                                                                              \n"
            "vec4 textureColor2 = texture2D(inputImageTextureLookup, vec2(1.0 - textureCoordinate.x,1.0 - textureCoordinate.y));                 \n"
            "vec4 textureColor1 = texture2D(inputImageTexture, textureCoordinate);                       \n"
            "float mask = textureColor2.r;                                                               \n"
         //   "vec4 newcolor1 ;            \n"
      //  "vec4 sourceColor = texture2D(inputImageTexture, textureCoordinate);\n"
        "vec4 sourceColor = textureColor1; \n"
        "float fstep = 0.0045;\n"
  //      "float ystep = fstep * 1280.0/720.0 ;\n"
        "float ystep = fstep; \n"
        "float scalePercent = 1.9; \n"
        "vec2 textureCoordinateToUse = textureCoordinate; \n"
        "vec2 center = vec2(0.5, 0.5); \n"
        "textureCoordinateToUse -= center; \n"
        "textureCoordinateToUse = textureCoordinateToUse / ((scalePercent-1.0)*0.6+1.0); \n"
        "textureCoordinateToUse += center; \n"
        "vec4 textureColor = texture2D(inputImageTextureLookup, textureCoordinateToUse); \n"
        "vec4 line2 = textureColor; \n"
        "float edgemask = textureColor.r;                                                               \n"


            #if 0
        "vec2 tex1 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y);\n"
        "vec4 p1 = texture2D(inputImageTexture, tex1);\n"
        "vec2 tex2 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y);\n"
        "vec4 p2 = texture2D(inputImageTexture, tex2);\n"
        "vec2 tex3 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y);\n"
        "vec4 p3 = texture2D(inputImageTexture, tex3);\n"
        "vec2 tex4 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y);\n"
        "vec4 p4 = texture2D(inputImageTexture, tex4);\n"
        "vec4 line7 = (p2 +  p1 +  sourceColor +  p3 + p4)/5.0;                            \n"
            #endif
            "vec4 line0 = newcolor(fstep,inputImageTexture,textureCoordinate); \n"
            #if 0
            "vec2 tex0 = vec2(textureCoordinate.x ,textureCoordinate.y + ystep );\n"
       "vec4 p0 = texture2D(inputImageTexture, tex0);\n"
       "vec2 tex5 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y + ystep);\n"
       "vec4 p5 = texture2D(inputImageTexture, tex5);\n"
       "vec2 tex6 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y + ystep);\n"
       "vec4 p6 = texture2D(inputImageTexture, tex6);\n"
       "vec2 tex7 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y + ystep);\n"
       "vec4 p7 = texture2D(inputImageTexture, tex7);\n"
       "vec2 tex8 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y + ystep);\n"
       "vec4 p8 = texture2D(inputImageTexture, tex8);\n"
       "vec4 line1 = (p6 +  p5 +  p0 +  p7 + p8)/5.0;                            \n"
            #endif
        //    "vec4 line1 = line0; \n"
#if 0

       "vec2 te0 = vec2(textureCoordinate.x ,textureCoordinate.y + 2.0 * ystep);\n"
       "vec4 pl0 = texture2D(inputImageTexture, te0);\n"
       "vec2 te5 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y + 2.0 * ystep);\n"
       "vec4 pl5 = texture2D(inputImageTexture, te5);\n"
       "vec2 te6 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y + 2.0 * ystep);\n"
       "vec4 pl6 = texture2D(inputImageTexture, te6);\n"
       "vec2 te7 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y + 2.0 * ystep);\n"
       "vec4 pl7 = texture2D(inputImageTexture, te7);\n"
       "vec2 te8 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y + 2.0 * ystep);\n"
       "vec4 pl8 = texture2D(inputImageTexture, te8);\n"
       "vec4 line2 = (pl6 +  pl5 +  pl0 + pl7 + pl8)/5.0;                            \n"
            #endif
      //      "vec4 line2 = line0; \n"
#if 0
       "vec2 t0 = vec2(textureCoordinate.x ,textureCoordinate.y -  ystep);\n"
       "vec4 pr0 = texture2D(inputImageTexture, t0);\n"
       "vec2 t5 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y - ystep);\n"
       "vec4 pr5 = texture2D(inputImageTexture, t5);\n"
       "vec2 t6 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y -  ystep);\n"
       "vec4 pr6 = texture2D(inputImageTexture, t6);\n"
       "vec2 t7 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y - ystep);\n"
       "vec4 pr7 = texture2D(inputImageTexture, t7);\n"
       "vec2 t8 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y - ystep);\n"
       "vec4 pr8 = texture2D(inputImageTexture, t8);\n"
       "vec4 line3 = (pr6 +  pr5 +  pr0 +  pr7 + pr8)/5.0;                            \n"
            #endif
    //        "vec4 line3 = line0; \n"

#if 0
       "vec2 tx0 = vec2(textureCoordinate.x ,textureCoordinate.y - 2.0 * ystep);\n"
       "vec4 px0 = texture2D(inputImageTexture, tx0);\n"
       "vec2 tx5 = vec2(textureCoordinate.x - 1.0 * fstep,textureCoordinate.y - 2.0 * ystep);\n"
       "vec4 px5 = texture2D(inputImageTexture, tx5);\n"
       "vec2 tx6 = vec2(textureCoordinate.x - 2.0 * fstep,textureCoordinate.y - 2.0 * ystep);\n"
       "vec4 px6 = texture2D(inputImageTexture, tx6);\n"
       "vec2 tx7 = vec2(textureCoordinate.x + 1.0 * fstep,textureCoordinate.y - 2.0 * ystep);\n"
       "vec4 px7 = texture2D(inputImageTexture, tx7);\n"
       "vec2 tx8 = vec2(textureCoordinate.x + 2.0 * fstep,textureCoordinate.y - 2.0 * ystep);\n"
       "vec4 px8 = texture2D(inputImageTexture, tx8 );\n"
       "vec4 line4 = (px6 + px5 +  px0 +  px7 + px8)/5.0;                            \n"
#endif
   //    "vec4 line4 = line0; \n"

            //    " if (sourceColor.r > 95.0/255.0 && sourceColor.g > 40.0/255.0 && sourceColor.b > 20.0/255.0 && sourceColor.r > sourceColor.b && sourceColor.r > sourceColor.g && abs(sourceColor.r - sourceColor.g ) > 15.0/255.0 ) {    \n"

   //    "vec4 p = ( line4 + 4.0 * line3 + 6.0 * line0 + 4.0 * line1 + line2)/16.0;\n"
   //    "vec4 p = ( line4 + line3 +  line0 +  line1 + line2)/5.0;\n"
            "float cstep = 0.0025; \n"
            "vec4 line1 = newcolor(cstep,inputImageTexture,textureCoordinate); \n"

            "float estep = 0.01; \n"
       //     "vec4 edgeline = newcolor(estep,inputImageTexture,textureCoordinate); \n"

            "if(edgemask == 0. && mask == 0. && enablemask == 1.)                                                       \n"
            "gl_FragColor = line0;                                                                     \n"

            "else if(edgemask > 0. && mask == 0. && enablemask == 1.) {                                                        \n"
    //        " if (sourceColor.r > 95.0/255.0 && sourceColor.g > 40.0/255.0 && sourceColor.b > 20.0/255.0 && sourceColor.r > sourceColor.b && sourceColor.r > sourceColor.g && abs(sourceColor.r - sourceColor.g ) > 15.0/255.0 ) {    \n"

            "gl_FragColor = line0;                                                                     \n"
   //         "}                                                                                           \n"
        "}                                                                                           \n"
     
            "else if ( mask > 0. && mask <= 0.95 && enablemask == 1.) gl_FragColor = line1;          \n"
            "else if ( mask > 0.95 && enablemask == 1.) gl_FragColor = textureColor1;                    \n"
            "else if ( enablemask == 0.) gl_FragColor = textureColor1;                                    \n"
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
        NormalFilter(int width, int height ,const char *fragment ) : FrameBuffer(width, height, DEFAULT_VERTEX_SHADER,fragment) {}
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 257, 257, 0, GL_RGBA, GL_UNSIGNED_BYTE, filter_buffer);
        glBindTexture(GL_TEXTURE_2D, 0);

    }

    void UpdateLut(uint8_t* lut, int width, int height) {
        glBindTexture(GL_TEXTURE_2D, filter_texture_id_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 257, 257, 0, GL_RGBA, GL_UNSIGNED_BYTE, lut);
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    void SetIntensity(float intensity = 1.0f) {
        intensity_ = intensity;
    }

     void SetEnableMask(float enablemask ) {
        enablemask_ = enablemask;
    }

    void SetPosition(float* position ) {
        position_ = position;
        plx_ = * position;
        ply_ = * (position + 1);
        Rx_ = * (position + 2);
        Ry_ = * (position + 3);

    }

    //create session for body segment
    void SetPortrait( int width , int height ) {
        auto net = MNN::Express::Variable::getInputAndOutput(MNN::Express::Variable::loadMap("/sdcard/data/deep.mnn"));
        if (!net.first.empty()) {

            //start setting output and
            input = net.first.begin()->second;
            auto info = input->getInfo();
            if (nullptr == info) {
                MNN_ERROR("The model don't have init dim\n");
                return;
            }
            auto shape = input->getInfo()->dim;
            shape[0] = 1;
            input->resize(shape);

            //int width = 257,height = 257;
            int bpp = 0;
            if (info->order == MNN::Express::NHWC) {
                bpp = shape[3];
                size_h = shape[1];
                size_w = shape[2];
            } else {
                bpp = shape[1];
                size_h = shape[2];
                size_w = shape[3];
            }
            if (bpp == 0)
                bpp = 1;
            if (size_h == 0)
                size_h = 1;
            if (size_w == 0)
                size_w = 1;
            // Set scale, from dst scale to src

            // Set scale, from dst scale to src
            trans.setScale((float) (width - 1) / (size_w - 1), (float) (height - 1) / (size_h - 1));
            config.filterType = MNN::CV::BILINEAR;
            //        float mean[3]     = {103.94f, 116.78f, 123.68f};
            //        float normals[3] = {0.017f, 0.017f, 0.017f};
            float mean[3] = {127.5f, 127.5f, 127.5f};
            float normals[3] = {0.00785f, 0.00785f, 0.00785f};
            ::memcpy(config.mean, mean, sizeof(mean));
            ::memcpy(config.normal, normals, sizeof(normals));
            config.sourceFormat = MNN::CV::ImageFormat::RGBA;
            config.destFormat = MNN::CV::ImageFormat::RGB;
            config.sourceFormat = MNN::CV::ImageFormat::RGBA;

            output = net.second.begin()->second;
            if (nullptr == output->getInfo()) {
                MNN_ERROR("Alloc memory or compute size error\n");
                return;
            }
            pretreatPtr = MNN::CV::ImageProcess::create(config);

            pretreatPtr->setMatrix(trans);


        } else {
            MNN_ERROR("Invalid Model\n");
        }
    }

    void SetRotate( unsigned char* inputImage, int width, int height,int channel,unsigned char* pMask, int dw) {

        MNN::CV::Matrix trans;
        //   trans.setScale((float)(width-1) / (dw-1), (float)(height-1) / (dw-1));

        trans.setScale(1.0 / (width - 1.0), 1.0 / (height - 1.0));
        trans.postRotate(90, 0.5, 0.5);
        trans.postScale((width - 1.0), (height - 1.0));

        MNN::CV::ImageProcess::Config config;
        config.filterType = MNN::CV::NEAREST;
        //  config.sourceFormat = MNN::CV::RGBA;
        config.sourceFormat = MNN::CV::YUV_NV21;
        config.destFormat = MNN::CV::RGBA;
        config.wrap = MNN::CV::ZERO;

        std::shared_ptr<MNN::CV::ImageProcess> pretreat(MNN::CV::ImageProcess::create(config));
        pretreat->setMatrix(trans);

    }
    void SetResize( int width, int height, int dw) {
        MNN::CV::Matrix trans;
        trans.setScale((float)(width-1) / (dw-1), (float)(height-1) / (dw-1));

        MNN::CV::ImageProcess::Config config;
        config.filterType   = MNN::CV::BILINEAR;
        config.sourceFormat = MNN::CV::YUV_NV21;
        config.destFormat   = MNN::CV::RGBA;
        config.wrap         = MNN::CV::ZERO;

        //  std::shared_ptr<MNN::CV::ImageProcess> pretreat(MNN::CV::ImageProcess::create(config));
        pretreat = MNN::CV::ImageProcess::create(config);
        pretreat->setMatrix(trans);


    }
    void GetResize( unsigned char* inputImage, int width, int height,int channel,unsigned char* pMask, int dw) {
#if 0
    MNN::CV::Matrix trans;
    trans.setScale((float)(width-1) / (dw-1), (float)(height-1) / (dw-1));

    MNN::CV::ImageProcess::Config config;
    config.filterType   = MNN::CV::BILINEAR;
    config.sourceFormat = MNN::CV::YUV_NV21;
    config.destFormat   = MNN::CV::RGBA;
    config.wrap         = MNN::CV::ZERO;

  //  std::shared_ptr<MNN::CV::ImageProcess> pretreat(MNN::CV::ImageProcess::create(config));
    pretreat = MNN::CV::ImageProcess::create(config);
    pretreat->setMatrix(trans);
#endif
    std::shared_ptr<MNN::Tensor> wrapTensor(MNN::CV::ImageProcess::createImageTensor<uint8_t>(dw, dw, 4, nullptr));
    pretreat->convert((uint8_t*)inputImage, width, height, 0, wrapTensor.get());

    memcpy( pMask , wrapTensor->host<uint8_t>() , sizeof(unsigned char) * dw * dw * 4);
/*
        for (int y = 0; y < dw; ++y) {
        unsigned char* outY = pMask + 4 * y * dw;
        unsigned char* rY = wrapTensor->host<uint8_t>() + 4 * y * dw ;
        memcpy( outY , rY , sizeof(unsigned char) * dw * 4);
    }
    */

#if 0
    for (int y = 0; y < height; ++y) {
        unsigned char* outY = pMask + 4 * y * width;
        unsigned char* rY = wrapTensor->host<uint8_t>() + 4 * y * width ;
        memcpy( outY , rY , sizeof(unsigned char) * width * 4);
    }
#endif

}

    //execut session for body segment and get output
void GetPortrait( unsigned char* inputImage, int width, int height,unsigned char* pMask) {


            pretreatPtr->convert((uint8_t*)inputImage, width, height, 0, input->writeMap<float>(), size_w, size_h, 4, 0, halide_type_of<float>());
    
            //auto originOrder = output->getInfo()->order;
            auto outputnh = _Convert(output, MNN::Express::NHWC);
            //output = _Softmax(output, -1);
            auto outputInfo = outputnh->getInfo();
            auto outw = outputInfo->dim[2];
            auto outh = outputInfo->dim[1];
            auto outch = outputInfo->dim[3];
            std::shared_ptr<MNN::Tensor> wrapTensor(MNN::CV::ImageProcess::createImageTensor<uint8_t>(width, height, 4, nullptr));
            //    MNN_PRINT("Mask: w=%d, h=%d, c=%d\n", width, height, channel);

            auto outputHostPtr = outputnh->readMap<float>();
            for (int y = 0; y < outh; ++y) {
                auto rgbaY = wrapTensor->host<uint8_t>() + 4 * y * outw;
                auto sourceY = outputHostPtr + y * outw * outch;
                for (int x = 0; x < outw; ++x) {
                    auto sourceX = sourceY + outch * x;
                    int index = 0;
                    float maxValue = sourceX[0];
                    auto rgba = rgbaY + 4 * x;
                    for (int c = 1; c < outch; ++c) {
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
                        rgba[2] = 255;
                        rgba[1] = 255;
                        rgba[3] = 0;

                    }
                }
            }
#if 1
            for (int y = 0; y < outh; ++y) {
                auto outY = pMask + 4 * y * outw;
                auto rY = wrapTensor->host<uint8_t>() + 4 * y * outw ;
                memcpy( outY , rY , sizeof(unsigned char) * outw * 4);
            }
#endif
            output->unMap();
            //stbir_resize_uint8( wrapTensor->host<uint8_t>() , outw , outh , 0, output_pixels, out_rw, out_rh, 0, 4);
        }
/*
   //execute inference to get portrait
    void pretreat(){

    }
*/
    ~NormalFilter() {
        if (filter_texture_id_ != 0) {
            glDeleteTextures(1, &filter_texture_id_);
            filter_texture_id_ = 0;
        }
    }

 private:
    GLuint filter_texture_id_;
    GLuint mask_texture_id_;
    float intensity_;
    float enablemask_ = 0.;
    float *position_;
    float plx_ = 0.;
    float ply_ = 0.;
    float Rx_ = 0.;
    float Ry_ = 0.;
  //  std::pair<std::map<std::string, MNN::Express::VARP>, std::map<std::string, MNN::Express::VARP>> &net;
    MNN::CV::ImageProcess::Config config;
    MNN::CV::ImageProcess* pretreatPtr;
    MNN::Express::VARP input;
    MNN::Express::VARP output;
    MNN::Express::VARP outputPtr;
    MNN::Express::Variable* netPtr;
    MNN::CV::Matrix trans;
    int size_w = 0;
    int size_h = 0;
    MNN::CV::ImageProcess* pretreat;

  //  unsigned char* pMask; //portrait mask for get body

 protected:
    virtual void RunOnDrawTasks() {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, filter_texture_id_);
        SetInt("inputImageTextureLookup", 1);
        SetFloat("intensity", intensity_);
        SetFloat("enablemask", enablemask_);
 
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
    Filter(int width, int height) : NormalFilter(width, height, ROTATE_FRAGMENT_SHADER) {}

    Filter(int width, int height,int type) : NormalFilter(width, height, MASK_FRAGMENT_SHADER) {}

    Filter(uint8_t* filter_buffer,int width, int height, float* position) : NormalFilter( filter_buffer, width, height,MASK_FRAGMENT_SHADER) {}

    Filter(uint8_t* filter_buffer, int width, int height, int ftype) : NormalFilter( filter_buffer, width, height, MASK_FRAGMENT_SHADER) {}

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
