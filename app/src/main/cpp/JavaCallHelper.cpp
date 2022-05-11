//
// jnicen
// Created by yonglin on 2021/5/26.
//

#include "JavaCallHelper.h"
#include "constant.h"

JavaCallHelper::JavaCallHelper(JavaVM *javaVm, JNIEnv *env, jobject instance) {

    //涉及线程切换问题  主线程回调
    this->javaVm = javaVm;
    this->env = env;
    //涉及jobject方法 跨方法 跨线程 创建全局引用
    this->instance = env->NewGlobalRef(instance);
    jclass objectClass = env->GetObjectClass(instance);
    this->onErrorMethod = env->GetMethodID(objectClass, "onError","(I)V");
    this->onPrepareMethod = env->GetMethodID(objectClass, "onPrepare","()V");

}

JavaCallHelper::~JavaCallHelper() {
    env->DeleteGlobalRef(instance);
}

void JavaCallHelper::onError(int thread,int errorCode){
    //主线程
    if (thread == THREAD_MAIN){
        env->CallVoidMethod(instance,onErrorMethod,errorCode);
    } else{
        //子线程
        JNIEnv *env;
        //获得属于当前线程的env
        javaVm->AttachCurrentThread(&env,0);
        //使用获得的env调用方法
        env->CallVoidMethod(instance,onErrorMethod,errorCode);
        //调用完之后分离
        javaVm->DetachCurrentThread();
    }
}

void JavaCallHelper::onPrepare(int thread) {
//主线程
    if (thread == THREAD_MAIN){
        env->CallVoidMethod(instance,onPrepareMethod);
    } else{
        //子线程
        JNIEnv *env;
        //获得属于当前线程的env
        javaVm->AttachCurrentThread(&env,0);
        //使用获得的env调用方法
        env->CallVoidMethod(instance,onPrepareMethod);
        //调用完之后分离
        javaVm->DetachCurrentThread();
    }
}
