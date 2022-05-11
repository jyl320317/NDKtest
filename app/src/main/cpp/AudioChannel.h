//
// Created by yonglin on 2021/5/26.
//

#ifndef NDKTEST_AUDIOCHANNEL_H
#define NDKTEST_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <pthread.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include <libswresample/swresample.h>
}

class AudioChannel : public BaseChannel {

public:
    uint8_t *data = 0;
    int out_channels;
    int out_samplesize;
    int out_sample_rate;

private:
    pthread_t pid_audio_play;
    pthread_t pid_audio_decode;


    /**
     * OpenSL ES
     */
    // 引擎与引擎接口
    SLObjectItf engineObject = 0;
    SLEngineItf engineInterface = 0;
    //混音器
    SLObjectItf outputMixObject = 0;
    //播放器
    SLObjectItf bqPlayerObject = 0;
    //播放器接口
    SLPlayItf bqPlayerInterface = 0;

    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = 0;

    //重采样
    SwrContext *swrContext = 0;

public:
    AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext, AVRational time_base);

    virtual ~AudioChannel();

    void play();

    void decode();

    void stop();

    void initOpenSL();

    void releaseOpenSL();

    int getPcm();
};


#endif //NDKTEST_AUDIOCHANNEL_H
