#include <android/bitmap.h>
#include <android/log.h>
#include <jni.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <string>
#include <vector>
#include "image_process.h"
//#include "../../../../../../../Library/Android/sdk/ndk/21.3.6528147/toolchains/llvm/prebuilt/darwin-x86_64/sysroot/usr/include/jni.h"

#define TAG "pabeauty"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

using namespace trinity;
static ImageProcess *image_process = nullptr;
int beauty_id = -1;

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_pasdk_PARenderer_setBeautificationOn(JNIEnv *env, jobject instance){

    if ( image_process == nullptr)
        image_process = new ImageProcess();

    beauty_id = image_process->OnBeautyFilter();

    return true;
}

JNIEXPORT jint JNICALL
Java_com_pasdk_PARenderer_onDraw(JNIEnv *env, jobject instance, jint tex , jint w , jint h ){


    if ( image_process == nullptr){
        image_process = new ImageProcess();
        beauty_id = image_process->OnBeautyFilter();
    }
    int idTex = image_process->Process( tex, w, h );
    return idTex;
}


JNIEXPORT jint JNICALL
Java_com_pasdk_PARenderer_onFilterSelected(JNIEnv *env, jobject instance ) {

    if ( image_process == nullptr)
        image_process = new ImageProcess();
    beauty_id = image_process->OnBeautyFilter();
    return beauty_id;

}

JNIEXPORT jboolean JNICALL
Java_com_pasdk_PARenderer_onDeleteAllFilters(JNIEnv *env, jobject instance){

    image_process->OnDeleteAllFilters() ;
    return true;
}

JNIEXPORT jboolean JNICALL
Java_com_pasdk_PARenderer_onDeleteOneFilter(JNIEnv *env, jobject instance,jint filter_id){

    image_process->OnDeleteFilter(filter_id) ;
    return true;
}

#if 0
JNIEXPORT jboolean JNICALL
Java_com_faceunity_FURenderer_FaceDetectionModelInit(JNIEnv *env, jobject instance,
                                                      jstring faceDetectionModelPath_) {
    LOGD("JNI init native sdk");
    if (detection_sdk_init_ok) {
        LOGD("sdk already init");
        return true;
    }
    jboolean tRet = false;
    if (NULL == faceDetectionModelPath_) {
        LOGD("model dir is empty");
        return tRet;
    }

    //获取模型的绝对路径的目录（不是/aaa/bbb.bin这样的路径，是/aaa/)
    const char *faceDetectionModelPath = env->GetStringUTFChars(faceDetectionModelPath_, 0);
    if (NULL == faceDetectionModelPath) {
        LOGD("model dir is empty");
        return tRet;
    }

    string tFaceModelDir = faceDetectionModelPath;
    string tLastChar = tFaceModelDir.substr(tFaceModelDir.length()-1, 1);
    //RFB-320
    //RFB-320-quant-ADMM-32
    //RFB-320-quant-KL-5792
    //slim-320
    //slim-320-quant-ADMM-50 
    //量化模型需要使用CPU方式 net.cpp中修改 sch_config.type = (MNNForwardType)MNN_FORWARD_CPU
    // change names
    string str = tFaceModelDir + "RFB-320-quant-ADMM-32.mnn";

    ultra = new  UltraFace(str, 320, 240, 4, 0.65 ); // config model input

    env->ReleaseStringUTFChars(faceDetectionModelPath_, faceDetectionModelPath);
    detection_sdk_init_ok = true;
    tRet = true;

    return tRet;
}

JNIEXPORT jintArray JNICALL
Java_com_facesdk_FaceSDKNative_FaceDetect(JNIEnv *env, jobject instance, jbyteArray imageDate_,
                                          jint imageWidth, jint imageHeight, jint imageChannel) {
    if(!detection_sdk_init_ok){
        LOGD("sdk not init");
        return NULL;
    }

    int tImageDateLen = env->GetArrayLength(imageDate_);
    if(imageChannel == tImageDateLen / imageWidth / imageHeight){
        LOGD("imgW=%d, imgH=%d,imgC=%d",imageWidth,imageHeight,imageChannel);
    }
    else{
        LOGD("img data format error");
        return NULL;
    }

    jbyte *imageDate = env->GetByteArrayElements(imageDate_, NULL);
    if (NULL == imageDate){
        LOGD("img data is null");
        return NULL;
    }

    if(imageWidth<200||imageHeight<200){
        LOGD("img is too small");
        return NULL;
    }


    std::vector<FaceInfo> face_info;
    //detect face
    ultra ->detect((unsigned char*)imageDate, imageWidth, imageHeight, imageChannel, face_info );

    int32_t num_face = static_cast<int32_t>(face_info.size());

    int out_size = 1+num_face*4;
    int *allfaceInfo = new int[out_size];
    allfaceInfo[0] = num_face;
    for (int i=0; i<num_face; i++) {

        allfaceInfo[4*i+1] = face_info[i].x1;//left
        allfaceInfo[4*i+2] = face_info[i].y1;//top
        allfaceInfo[4*i+3] = face_info[i].x2;//right
        allfaceInfo[4*i+4] = face_info[i].y2;//bottom

    }

    jintArray tFaceInfo = env->NewIntArray(out_size);
    env->SetIntArrayRegion(tFaceInfo, 0, out_size, allfaceInfo);
    env->ReleaseByteArrayElements(imageDate_, imageDate, 0);


    delete [] allfaceInfo;

    return tFaceInfo;
}

JNIEXPORT jboolean JNICALL
Java_com_facesdk_FaceSDKNative_FaceDetectionModelUnInit(JNIEnv *env, jobject instance) {

    jboolean tDetectionUnInit = false;

    if (!detection_sdk_init_ok) {
        LOGD("sdk not inited, do nothing");
        return true;
    }

    delete ultra;

    detection_sdk_init_ok = false;

    tDetectionUnInit = true;

    LOGD("sdk release ok");

    return tDetectionUnInit;
}
#endif
}
