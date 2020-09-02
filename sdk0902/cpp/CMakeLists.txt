cmake_minimum_required(VERSION 3.10.1)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(./include
                    ./include/MNN
                    ./include/stb
                    ./cpp/
                    ./includes
                    ./cpp/json)


#add_library(MNN SHARED IMPORTED)

#set_target_properties(MNN
#  PROPERTIES IMPORTED_LOCATION
#  ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libMNN.so)

add_library(pabeauty SHARED ./pabeauty_jni.cpp
        ./opengl.cc
        ${CMAKE_SOURCE_DIR}/frame_buffer.cc
        ${CMAKE_SOURCE_DIR}/image_process.cc)



find_library(log-lib log)
find_library( jnigraphics-lib jnigraphics )

#link_libraries(  MNN )
#add_definitions(-DMNN_USE_LOGCAT)
target_link_libraries(pabeauty  GLESv2 EGL OpenSLES)

#target_link_libraries(portraitdetect MNN GLESv2 EGL OpenSLES)
#设置要编译的源文件
#add_executable(mland.out ./mlandmark.cpp src/main/cpp/net.cpp )
#add_executable(seg.out ./segment.cpp src/main/cpp/net.cpp )
#add_executable(pose.out ./multiPose.cpp )
