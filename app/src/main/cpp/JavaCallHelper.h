//
// jni层回调Java层方法
// Created by yonglin on 2021/5/26.
//

#ifndef NDKTEST_JAVACALLHELPER_H
#define NDKTEST_JAVACALLHELPER_H

#include "jni.h"
class JavaCallHelper {

private:
    JavaVM *javaVm;
    JNIEnv *env;
    jobject instance;
    jmethodID onErrorMethod;
    jmethodID onPrepareMethod;
public:
    //构造方法
    JavaCallHelper(JavaVM *javaVm, JNIEnv *env, jobject instance);

    //析构方法
    ~JavaCallHelper();

    //播放数据准备好了通知java层进行播放
    void onPrepare(int thread);

    //播放器出错 回调java层处理
    void onError(int thread, int errorCode);
};


#endif //NDKTEST_JAVACALLHELPER_H
