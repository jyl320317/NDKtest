//
// Created by yonglin on 2021/5/26.
//

#include <cstring>
#include <pthread.h>
#include "FFmpeg.h"
#include "constant.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

//准备播放函数指针
void *task_prepare(void *args) {
    FFmpeg *ffmpeg = static_cast<FFmpeg *>(args);
    ffmpeg->_prepare();
    return 0;
}

void *aync_stop(void *args) {
    FFmpeg *ffmpeg = static_cast<FFmpeg *>(args);
    //   等待prepare结束
    pthread_join(ffmpeg->pid, 0);
    ffmpeg->isPlaying = 0;
    // 保证 start线程结束
    pthread_join(ffmpeg->pid_play, 0);
    DELETE(ffmpeg->videoChannel);
    DELETE(ffmpeg->audioChannel);
    // 这时候释放就不会出现问题了
    if (ffmpeg->formatContext) {
        //先关闭读取 (关闭fileintputstream)
        avformat_close_input(&ffmpeg->formatContext);
        avformat_free_context(ffmpeg->formatContext);
        ffmpeg->formatContext = 0;
    }
    DELETE(ffmpeg);
    LOGE("释放");
    return 0;
}

FFmpeg::FFmpeg(JavaCallHelper *callHelper, const char *dataSource) {
    this->callHelper = callHelper;
    //防止dataSource类型指向的内存被释放  悬空指针问题
    //c++字符串以/0结尾
    // strlen获取的是不包含/0的
    this->dataSource = new char[strlen(dataSource) + 1];
    strcpy(this->dataSource, dataSource);
    isPlaying = false;
    duration = 0;
    pthread_mutex_init(&seekMutex, 0);
}

FFmpeg::~FFmpeg() {
    //释放
    DELETE(dataSource)
    DELETE(callHelper)
    pthread_mutex_destroy(&seekMutex);
}

void FFmpeg::prepare() {
    //创建一个线程
    pthread_create(&pid, 0, task_prepare, this);
}

void FFmpeg::_prepare() {
    //ffmpeg允许使用网络
    avformat_network_init();
    //包含视频的宽高等信息
    formatContext = 0;
    //1、打开URL
    AVDictionary *opts = 0;
    //设置超时3秒
    av_dict_set(&opts, "timeout", "5000000", 0);
    //打开需要播放的媒体地址(文件、直播地址)  返回值int  打开成功or失败
    int result = avformat_open_input(&formatContext, dataSource, 0, &opts);
    //不为0打开失败
    if (result != 0) {
        LOGE("打开媒体失败%s", av_err2str(result));
        callHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        return;
    }
    //获取音视频流
    result = avformat_find_stream_info(formatContext, 0);
    //小于0获取流失败
    if (result < 0) {
        LOGE("获取流失败%s", av_err2str(result));
        callHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        return;
    }
    duration = formatContext->duration / 1000000;
    //nb_streams 代表几个流  几段视频  几段音频
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream *stream = formatContext->streams[i];
        //包含解码这段流的各种参数信息
        AVCodecParameters *codecpar = stream->codecpar;
        //对音视频的通用处理
        //1.获取解码器
        //通过当前流使用的编码方式  查找解码器
        AVCodec *avCodec = avcodec_find_decoder(codecpar->codec_id);
        if (avCodec == nullptr) {
            LOGE("获取解码器失败");
            callHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            return;
        }
        //获取解码器上下文
        AVCodecContext *context = avcodec_alloc_context3(avCodec);
        if (context == nullptr) {
            LOGE("获取解码器上下文失败");
            callHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }
        //设置上下文的一些参数
        result = avcodec_parameters_to_context(context, codecpar);
        if (result < 0) {
            LOGE("根据流信息设置上下文参数失败%s", av_err2str(result));
            callHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }

        //2.打开解码器
        result = avcodec_open2(context, avCodec, 0);
        if (result != 0) {
            LOGE("打开解码器失败%s", av_err2str(result));
            callHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            return;
        }
        // 单位
        AVRational time_base = stream->time_base;
        //音频
        if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioChannel = new AudioChannel(i, callHelper, context, time_base);
        } else if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            //视频
            //帧率： 单位时间内 需要显示多少个图像
            AVRational frame_rate = stream->avg_frame_rate;
            int fps = av_q2d(frame_rate);
            videoChannel = new VideoChannel(i, callHelper, context, time_base, fps);
            videoChannel->setRenderFrameCallback(callback);
        }
    }
    //没有音视频  (很少见)
    if (!audioChannel && !videoChannel) {
        LOGE("没有音视频");
        callHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
        return;
    }
    // 准备完了 通知java 你随时可以开始播放
    if (callHelper) {
        callHelper->onPrepare(THREAD_CHILD);
    }
}

/**
 * 开始播放的函数指针
 * @param args
 * @return
 */
void *play(void *args) {
    FFmpeg *ffmpeg = static_cast<FFmpeg *>(args);
    ffmpeg->_start();
    return 0;
}


void FFmpeg::start() {
    isPlaying = 1;
    //启动声音的解码与播放
    if (audioChannel) {
        audioChannel->play();
    }
    if (videoChannel) {
        videoChannel->setAudioChannel(audioChannel);
        videoChannel->play();
    }
    pthread_create(&pid_play, 0, play, this);
}

/**
 * 此线程用于专门读取数据包
 */
void FFmpeg::_start() {
    //1、读取媒体数据包
    int ret;
    while (isPlaying) {
        //读取文件的时候没有网络请求，一下子读完了，可能导致oom
        //特别是读本地文件的时候 一下子就读完了
        if (audioChannel && audioChannel->packets.size() > 100) {
            //10ms
            av_usleep(1000 * 10);
            continue;
        }
        if (videoChannel && videoChannel->packets.size() > 100) {
            av_usleep(1000 * 10);
            continue;
        }
        //锁住formatContext
        pthread_mutex_lock(&seekMutex);
        //AVPacket 存放编码数据
        //AVFrame  存放解码数据
        AVPacket *packet = av_packet_alloc();
        ret = av_read_frame(formatContext, packet);
        pthread_mutex_unlock(&seekMutex);
        //非0失败
        if (ret == 0) {
            //stream_index 流序号  判定是音频包还是视频包
            if (audioChannel && packet->stream_index == audioChannel->channelId) {
                //音频  放入到队列中
                audioChannel->packets.push(packet);
            } else if (videoChannel && packet->stream_index == videoChannel->channelId) {
                //视频  放入到队列中
                videoChannel->packets.push(packet);
            }
        } else if (ret == AVERROR_EOF) {
            //读取完成 可能还没有播放完
            if (audioChannel->packets.empty() && audioChannel->frames.empty() &&
                videoChannel->packets.empty() && videoChannel->frames.empty()) {
                break;
            }
            //因为seek 的存在，就算读取完毕，依然要循环 去执行av_read_frame(否则seek了没用...)
        } else {
            break;
        }
    }
    isPlaying = 0;
    audioChannel->stop();
    videoChannel->stop();
}

void FFmpeg::setRenderFrameCallback(RenderFrameCallback callback) {
    this->callback = callback;
}

void FFmpeg::stop() {
    callHelper = 0;
    if (audioChannel) {
        audioChannel->javaCallHelper = 0;
    }
    if (videoChannel) {
        videoChannel->javaCallHelper = 0;
    }
    pthread_create(&pid_stop, 0, aync_stop, this);
}

void FFmpeg::seek(int i) {
    //进去必须 在0- duration 范围之类
    if (i < 0 || i >= duration) {
        return;
    }
    if (!audioChannel && !videoChannel) {
        return;
    }
    if (!formatContext) {
        return;
    }
    isSeek = 1;
    pthread_mutex_lock(&seekMutex);
    //单位是 微妙
    int64_t seek = i * 1000000;
    //seek到请求的时间 之前最近的关键帧
    // 只有从关键帧才能开始解码出完整图片
    av_seek_frame(formatContext, -1, seek, AVSEEK_FLAG_BACKWARD);
//    avformat_seek_file(formatContext, -1, INT64_MIN, seek, INT64_MAX, 0);
    // 音频、与视频队列中的数据 是不是就可以丢掉了？
    if (audioChannel) {
        //暂停队列
        audioChannel->stopWork();
        //可以清空缓存
//        avcodec_flush_buffers();
        audioChannel->clear();
        //启动队列
        audioChannel->startWork();
    }
    if (videoChannel) {
        videoChannel->stopWork();
        videoChannel->clear();
        videoChannel->startWork();
    }
    pthread_mutex_unlock(&seekMutex);
    isSeek = 0;
}
