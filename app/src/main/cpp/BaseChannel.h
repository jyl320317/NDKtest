//
// 抽取AudioChannel VideoChannel共同点
// id 流序号
// Created by yonglin on 2021/5/27.
//

#ifndef NDKTEST_BASECHANNEL_H
#define NDKTEST_BASECHANNEL_H

#include "safe_queue.h"
#include "JavaCallHelper.h"
#include "constant.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

class BaseChannel {

public:
    volatile int channelId;
    volatile bool isPlaying;
    AVCodecContext *avCodecContext = 0;
    //编码数据包队列
    //存放队列中的是指针  存放在堆中  需要手动释放堆内存
    SafeQueue<AVPacket *> packets;

    //解码数据包队列
    SafeQueue<AVFrame *> frames;

    //时间基  相对开始播放
    AVRational time_base;
    double clock = 0;
    JavaCallHelper *javaCallHelper;

public:
    BaseChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
                AVRational time_base) :
            channelId(id),
            javaCallHelper(javaCallHelper),
            avCodecContext(avCodecContext),
            time_base(time_base) {
        packets.setReleaseCallBack(BaseChannel::releaseAVPacket);
        frames.setReleaseCallBack(BaseChannel::releaseAVFrame);
    }

    /**
     * 虚方法的析构函数
     * 子类继承父类  使用虚方法为了使子类能够调用自己的析构函数
     */
    virtual ~BaseChannel() {
        if (avCodecContext) {
            avcodec_close(avCodecContext);
            avcodec_free_context(&avCodecContext);
            avCodecContext = 0;
        }
        packets.clear();
        frames.clear();
        LOGE("释放channel:%d %d", packets.size(), frames.size());
    }

    /**
     * 释放 AVPacket
     * 为什么用指针的指针？
     * 指针的指针可以修改传递进来的指针的指向
     * @param pPacket
     */
    static void releaseAVPacket(AVPacket **pPacket) {
        if (pPacket) {
            av_packet_free(pPacket);
            *pPacket = 0;
        }
    }

    /**
     * 释放AVFrame
     * @param pAvFrame
     */
    static void releaseAVFrame(AVFrame **pAvFrame) {
        if (pAvFrame) {
            av_frame_free(pAvFrame);
            *pAvFrame = 0;
        }
    }

    void clear() {
        packets.clear();
        frames.clear();
    }

    void stopWork() {
        packets.setWork(0);
        frames.setWork(0);
    }

    void startWork() {
        packets.setWork(1);
        frames.setWork(1);
    }

    /**
     * 纯虚方法 相当于抽象方法
     */
    virtual void play() = 0;

    virtual void stop() = 0;
};


#endif //NDKTEST_BASECHANNEL_H
