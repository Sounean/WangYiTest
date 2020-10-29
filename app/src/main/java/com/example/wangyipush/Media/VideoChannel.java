package com.example.wangyipush.Media;

import android.app.Activity;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;

import com.example.wangyipush.LivePusher;

public class VideoChannel implements Camera.PreviewCallback, CameraHelper.OnChangedSizeListener {
    private static final String TAG = "VideoChannel";
    private CameraHelper cameraHelper;
    private int mBitrate;
    private int mFps;
    private boolean isLiving;
    LivePusher livePusher;
    public VideoChannel(LivePusher livePusher, Activity activity, int width, int height, int bitrate, int fps, int cameraId) {
        mBitrate = bitrate;
        mFps = fps;
        this.livePusher = livePusher;
        cameraHelper = new CameraHelper(activity, cameraId, width, height);
        cameraHelper.setPreviewCallback(this);
        cameraHelper.setOnChangedSizeListener(this);
    }

    /*
    *Camera.PreviewCallback会回调此方法，作用为返回摄像头的内容，内容都在data里
    * 当摄像头打开了就会被调用。
    * */
    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        Log.i(TAG, "onPreviewFrame: ");
        if (isLiving) {//如果是true则进行推流。
            Log.i(TAG, "push");
            livePusher.native_pushVideo(data);//这个data数据就是我们所说的一帧。
        }
    }

    @Override
    public void onChanged(int w, int h) {
        livePusher.native_setVideoEncInfo(w, h, mFps, mBitrate);
    }
    public void switchCamera() {
        cameraHelper.switchCamera();
    }

    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        cameraHelper.setPreviewDisplay(surfaceHolder);
    }

    public void startLive() {
        isLiving = true;
    }

    public void stopLive() {
        isLiving = false;
    }
}
