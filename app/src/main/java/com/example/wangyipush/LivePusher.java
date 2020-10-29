package com.example.wangyipush;


import android.app.Activity;
import android.view.SurfaceHolder;


import com.example.wangyipush.Media.AudioChannel;
import com.example.wangyipush.Media.VideoChannel;

public class LivePusher {
    private AudioChannel audioChannel;
    private VideoChannel videoChannel;
    static {
        System.loadLibrary("native-lib");
    }

    public LivePusher(Activity activity, int width, int height, int bitrate,
                      int fps, int cameraId) {
        //对native层的VideoChannel进行初始化
        native_init();
        videoChannel = new VideoChannel(this, activity, width, height, bitrate, fps, cameraId);
        audioChannel = new AudioChannel(this);
    }

    public native void native_init();//初始化音、视频流
    public native void native_setVideoEncInfo(int w, int h, int mFps, int mBitrate);//初始化视频数据
    public native void native_start(String path);//开始直播
    public native void native_pushVideo(byte[] data);//开始数据编码
    public native void native_pushAudio(byte[] bytes);//音频推流部分
    public native void native_setAudioEncInfo(int i, int channels);
    public native int getInputSamples();

    /**
     * 设置摄像头预览
     * @param surfaceHolder
     */
    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        videoChannel.setPreviewDisplay(surfaceHolder);
    }


    /*
    * 开始直播
    * */
    public void startLive(String path) {
        native_start(path);
        videoChannel.startLive();
        audioChannel.startLive();//这里没有让它开始导致网页端哪里显示音频流是空的。
    }

    /**
     * 切换摄像头
     *//*
    public void switchCamera() {
        videoChannel.switchCamera();
    }

    *//**
     * 停止直播
     *//*
    public void stopLive() {
        videoChannel.stopLive();
        audioChannel.stopLive();
    }*/

  /*  public native void native_init();

    public native void native_setVideoEncInfo(int w, int h, int mFps, int mBitrate);



    public native void native_pushVideo(byte[] data);

    public native void native_setAudioEncInfo(int i, int channels);



    */

}

