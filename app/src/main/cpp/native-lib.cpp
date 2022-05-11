#include <jni.h>
#include <string>
#include <android/log.h>
#include "FFmpeg.h"
#include <android/native_window_jni.h>

// __VA_ARGS__ 代表 ...的可变参数
#define TAG "native-lib"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG,  __VA_ARGS__);
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG,  __VA_ARGS__);
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG,  __VA_ARGS__);


FFmpeg *ffmpeg = nullptr;
JavaVM *javaVm = nullptr;
ANativeWindow *window = nullptr;
JavaCallHelper *javaCallHelper = nullptr;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int JNI_OnLoad(JavaVM *vm, void *r) {
    javaVm = vm;
    return JNI_VERSION_1_6;
}

//画画
void render(uint8_t *data, int lineszie, int w, int h) {
    pthread_mutex_lock(&mutex);
    if (!window) {
        pthread_mutex_unlock(&mutex);
        return;
    }
    //设置窗口属性
    ANativeWindow_setBuffersGeometry(window, w,
                                     h,
                                     WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        ANativeWindow_release(window);
        window = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }
    //填充rgb数据给dst_data
    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    // stride：一行多少个数据（RGBA） *4
    int dst_linesize = window_buffer.stride * 4;
    //一行一行的拷贝
    for (int i = 0; i < window_buffer.height; ++i) {
        //memcpy(dst_data , data, dst_linesize);
        memcpy(dst_data + i * dst_linesize, data + i * lineszie, dst_linesize);
    }
    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}

extern "C" JNIEXPORT void JNICALL
Java_com_jyl_ndktest_player_Player_native_1prepare(JNIEnv *env, jobject thiz, jstring data_source) {

    const char *dataSource = env->GetStringUTFChars(data_source, 0);
    //创建播放器
    javaCallHelper = new JavaCallHelper(javaVm, env, thiz);
    ffmpeg = new FFmpeg(javaCallHelper, dataSource);
    ffmpeg->setRenderFrameCallback(render);
    ffmpeg->prepare();
    env->ReleaseStringUTFChars(data_source, dataSource);
}

extern "C" JNIEXPORT void JNICALL
Java_com_jyl_ndktest_player_Player_native_1start(JNIEnv *env, jobject thiz) {
    if (ffmpeg){
        ffmpeg->start();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jyl_ndktest_player_Player_native_1setSurface(JNIEnv *env, jobject thiz, jobject surface) {
    pthread_mutex_lock(&mutex);
    //释放旧的window
    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    window = ANativeWindow_fromSurface(env, surface);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jyl_ndktest_player_Player_native_1stop(JNIEnv *env, jobject thiz) {
    if (ffmpeg){
        ffmpeg->stop();
        ffmpeg = 0;
    }
    if (javaCallHelper){
        delete javaCallHelper;
        javaCallHelper = 0;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jyl_ndktest_player_Player_native_1release(JNIEnv *env, jobject thiz) {
    pthread_mutex_lock(&mutex);
    if (window) {
        //把老的释放
        ANativeWindow_release(window);
        window = 0;
    }
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jyl_ndktest_player_Player_native_1getDuration(JNIEnv *env, jobject thiz) {
    if (ffmpeg){
        return ffmpeg->getDuration();
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jyl_ndktest_player_Player_native_1seek(JNIEnv *env, jobject thiz, jint progress) {
    if (ffmpeg){
        ffmpeg->seek(progress);
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_jyl_ndktest_MainActivity_stringFromJni(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
//    std::string ffmpeg = avcodec_configuration();
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_jyl_ndktest_MainActivity_test1(JNIEnv *env, jobject thiz, jboolean b, jbyte b1, jchar c,
                                        jshort s, jlong l, jfloat f, jdouble d, jstring name,
                                        jint age, jintArray i, jobjectArray strs, jobject person,
                                        jbooleanArray b_array) {

    //1\. 接收 Java 传递过来的 boolean 值
    unsigned char b_boolean = b;
    LOGD("boolean-> %d", b_boolean)

    //2\. 接收 Java 传递过来的 byte 值
    char c_byte = b1;
    LOGD("jbyte-> %d", c_byte)

    //3\. 接收 Java 传递过来的 char 值
    unsigned short c_char = c;
    LOGD("char-> %d", c_char)

    //4\. 接收 Java 传递过来的 short 值
    short s_short = s;
    LOGD("short-> %d", s_short)

    //5\. 接收 Java 传递过来的 long 值
    long l_long = l;
    LOGD("long-> %ld", l_long)

    //6\. 接收 Java 传递过来的 float 值
    float f_float = f;
    LOGD("float-> %f", f_float)

    //7\. 接收 Java 传递过来的 double 值
    double d_double = d;
    LOGD("double-> %f", d_double)

    //8\. 接收 Java 传递过来的 String 值
    const char *name_string = env->GetStringUTFChars(name, 0);
    LOGD("string-> %s", name_string)

    //9\. 接收 Java 传递过来的 int 值
    int age_java = age;
    LOGD("int:%d", age_java)

    //10\. 打印 Java 传递过来的 int []
    jint *intArray = env->GetIntArrayElements(i, nullptr);
    //拿到数组长度
    jsize intArraySize = env->GetArrayLength(i);
    for (int i = 0; i < intArraySize; ++i) {
        LOGD("intArray->%d", intArray[i])
    }
    env->ReleaseIntArrayElements(i, intArray, 0);

    //11\. 打印 Java 传递过来的 String[]
    jsize stringArrayLength = env->GetArrayLength(strs);
    for (int i = 0; i < stringArrayLength; ++i) {
        jobject jobject1 = env->GetObjectArrayElement(strs, i);
        //强转 JNI String
        jstring stringArrayData = static_cast<jstring >(jobject1);

        //转 C  String
        const char *itemStr = env->GetStringUTFChars(stringArrayData, nullptr);
        LOGD("String[%d]: %s", i, itemStr);
        //回收 String[]
        env->ReleaseStringUTFChars(stringArrayData, itemStr);
    }

    //12\. 打印 Java 传递过来的 Object 对象
    //12.1 获取字节码
    const char *person_class_str = "com/jyl/ndktest/bean/Person";
    //12.2 转 jni jclass
    jclass person_class = env->FindClass(person_class_str);
    jmethodID jmethodID1 = env->GetMethodID(person_class, "getName", "()Ljava/lang/String;");
    jobject obj_string = env->CallObjectMethod(person, jmethodID1);
    jstring perStr = static_cast<jstring >(obj_string);
    const char *itemStr2 = env->GetStringUTFChars(perStr, nullptr);
    LOGD("Person: %s", itemStr2);
    jmethodID jmethodID2 = env->GetMethodID(person_class, "getAge", "()I");
    jint callIntMethod = env->CallIntMethod(person, jmethodID2);
    LOGD("Person age: %d", callIntMethod)
    env->DeleteLocalRef(person_class); // 回收
    env->DeleteLocalRef(person); // 回收

    //13\. 打印 Java 传递过来的 booleanArray
    jsize booArrayLength = env->GetArrayLength(b_array);
    jboolean *bArray = env->GetBooleanArrayElements(b_array, nullptr);
    for (int i = 0; i < booArrayLength; ++i) {
        bool b = bArray[i];
        jboolean b2 = bArray[i];
        LOGD("boolean:%d", b)
        LOGD("jboolean:%d", b2)
    }
    //回收
    env->ReleaseBooleanArrayElements(b_array, bArray, 0);

}