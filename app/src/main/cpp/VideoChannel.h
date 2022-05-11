//
// Created by yonglin on 2021/5/26.
//

#ifndef NDKTEST_VIDEOCHANNEL_H
#define NDKTEST_VIDEOCHANNEL_H

#include "BaseChannel.h"
#include "AudioChannel.h"

extern "C" {
#include <libswscale/swscale.h>
}

/**
 *  解码
 *  播放
 */
typedef void (*RenderFrameCallback)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel {
public:
    VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext, AVRational time_base, int fps);

    ~VideoChannel();

    void play();

    void decode();

    void stop();

    void render();

    void setRenderFrameCallback(RenderFrameCallback callback);

    void setAudioChannel(AudioChannel* audioChannel);

private:
    //解码线程
    pthread_t pid_decode;
    //渲染线程
    pthread_t pid_render;
    SwsContext *swsContext=0;
    RenderFrameCallback callback;
    //帧率
    int fps;
    AudioChannel *audioChannel = 0;
};

#endif //NDKTEST_VIDEOCHANNEL_H
