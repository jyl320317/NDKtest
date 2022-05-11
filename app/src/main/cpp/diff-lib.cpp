#include <jni.h>

//
// Created by yonglin on 2021/11/4.
//

extern "C"{
    extern int main(int argc,char * argv[]);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jyl_ndktest_DiffActivity_bspatch(JNIEnv *env, jobject thiz, jstring oldapk, jstring patch,
                                          jstring output) {

    const char *oldapkpath = env->GetStringUTFChars(oldapk, 0);
    const char *patchpath = env->GetStringUTFChars(patch, 0);
    const char *outputpath = env->GetStringUTFChars(output, 0);

    char * argv[4] = {"", const_cast<char *>(oldapkpath), const_cast<char *>(outputpath),const_cast<char *>(patchpath)};
    main(4,argv);

    env->ReleaseStringUTFChars(oldapk,oldapkpath);
    env->ReleaseStringUTFChars(output,outputpath);
    env->ReleaseStringUTFChars(patch,patchpath);

}