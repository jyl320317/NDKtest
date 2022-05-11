package com.jyl.ndktest.player;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

import java.util.concurrent.Executors;

/**
 * 定制播放器  提供播放、停止等功能
 *
 * @author yonglin
 */
public class Player implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("native-lib");
    }

    private String dataSource;
    private SurfaceHolder holder;
    private OnPrepareListener prepareListener;
    private OnProgressListener onProgressListener;
    private OnErrorListener onErrorListener;

    /**
     * 设置播放源
     *
     * @param dataSource
     */
    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        if (holder != null){
            holder.removeCallback(this);
        }
        holder = surfaceView.getHolder();
        native_setSurface(holder.getSurface());
        holder.addCallback(this);
    }


    /**
     * 准备好播放的视频
     */
    public void prepare() {
        native_prepare(dataSource);
    }

    /**
     * 开始播放
     */
    public void start() {
        native_start();
    }

    /**
     * 停止播放
     */
    public void stop() {
        native_stop();
    }

    public int getDuration() {
        return native_getDuration();
    }

    public void seek(final int progress) {
        Executors.newSingleThreadExecutor().execute(() -> native_seek(progress));
    }

    /**
     * 释放资源
     */
    public void release() {
        if (holder != null){
            holder.removeCallback(this);
        }
        native_release();
    }

    /**
     * 画布创建
     *
     * @param holder
     */
    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {

    }

    /**
     * 画布改变
     *
     * @param holder
     */
    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        native_setSurface(holder.getSurface());
    }

    /**
     * 画布销毁
     *
     * @param holder
     */
    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        native_stop();
    }

    /**
     * native层已准备好播放视频
     */
    public void onPrepare() {
        if (prepareListener != null) {
            prepareListener.onPrepare();
        }
    }

    /**
     * native 回调给java 播放进度
     * @param progress
     */
    public void onProgress(int progress) {
        if (null != onProgressListener) {
            onProgressListener.onProgress(progress);
        }
    }

    /**
     * 播放器出错回调java层
     *
     * @param errorCode
     */
    public void onError(int errorCode) {
        stop();
        if (null != onErrorListener) {
            onErrorListener.onError(errorCode);
        }
        System.out.println("收到native层播放出错的代码 = " + errorCode);
    }

    public void setPrepareListener(OnPrepareListener prepareListener) {
        this.prepareListener = prepareListener;
    }

    public void setOnProgressListener(OnProgressListener onProgressListener) {
        this.onProgressListener = onProgressListener;
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    /**
     * native层准备好要播放的视频回调java层
     */
    public interface OnPrepareListener {
        void onPrepare();
    }

    /**
     * native回调播放进度到java层
     */
    public interface OnProgressListener {
        void onProgress(int progress);
    }

    /**
     * native出错回调Java
     */
    public interface OnErrorListener {
        void onError(int error);
    }

    /**
     * native层准备播放的视频
     *
     * @param dataSource
     */
    native void native_prepare(String dataSource);

    /**
     * native播放视频
     */
    native void native_start();

    /**
     * 将画布给到ndk处理
     * @param surface
     */
    native void native_setSurface(Surface surface);

    /**
     * native停止播放
     */
    native void native_stop();

    /**
     * native
     */
    native int native_getDuration();

    /**
     * native处理进度
     * @param progress
     */
    native void native_seek(int progress);

    /**
     * 释放相应的资源
     */
    native void native_release();

}
