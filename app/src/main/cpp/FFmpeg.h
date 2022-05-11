//
// Created by yonglin on 2021/5/26.
//

#ifndef NDKTEST_FFMPEG_H
#define NDKTEST_FFMPEG_H

#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"

extern "C" {
#include <libavformat/avformat.h>
}

class FFmpeg {

public:
    char *dataSource; //播放地址源
    pthread_t pid; //准备播放的线程
    pthread_t pid_play;//开始播放的线程
    pthread_t pid_stop;//停止播放的线程
    AVFormatContext *formatContext = 0; //包含视频的宽高等信息
    JavaCallHelper *callHelper; //jni回调java层
    AudioChannel *audioChannel = 0; //指针初始化必须赋默认值
    VideoChannel *videoChannel = 0;
    bool isPlaying; //是否正在播放
    RenderFrameCallback callback;
    pthread_mutex_t seekMutex;
    int duration;
    bool isSeek = 0;

public:
    FFmpeg(JavaCallHelper *callHelper, const char *dataSource);

    ~FFmpeg();

    void prepare();

    void _prepare();

    void start();

    void _start();

    void stop();

    void setRenderFrameCallback(RenderFrameCallback callback);

    int getDuration(){
        return duration;
    }

    void seek(int i);
};


#endif //NDKTEST_FFMPEG_H
