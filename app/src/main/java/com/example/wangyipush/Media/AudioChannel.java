package com.example.wangyipush.Media;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;


import com.example.wangyipush.LivePusher;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;



public class AudioChannel {
    private LivePusher mLivePusher;
    private AudioRecord audioRecord;//通过这个来录取麦克风的数据
    private int inputSamples;
    private int channels = 2;//双声道
    int channelConfig;
    int minBufferSize;
    private ExecutorService executor;
    private boolean isLiving;


    public AudioChannel(LivePusher livePusher) {
        executor = Executors.newSingleThreadExecutor();
        mLivePusher = livePusher;
        if (channels == 2) {
            channelConfig = AudioFormat.CHANNEL_IN_STEREO;
        } else {
            channelConfig = AudioFormat.CHANNEL_IN_MONO;
        }
        mLivePusher.native_setAudioEncInfo(44100, channels);//采样频率，通道数。 在这里进行初始化。
        minBufferSize = AudioRecord.getMinBufferSize(44100,
                channelConfig, AudioFormat.ENCODING_PCM_16BIT);

        //faac返回的当前输入采样率对应的采样个数,因为是用的16位，所以*2是byte长度
       inputSamples = mLivePusher.getInputSamples() * 2;

        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, 44100, channelConfig,
                AudioFormat.ENCODING_PCM_16BIT, Math.min(minBufferSize, inputSamples));//主麦克风，采样频率，双声道（这里上面多了几步来赋值是为了方便动态赋值），采样位数，采集数据需要的缓冲区的大小。
    }

    public void startLive() {
        isLiving = true;
        executor.submit(new AudioTeask());
    }





    public void setChannels(int channels) {
        this.channels = channels;
    }


    /*
    * 通过线程读取麦克风的数据
    * */
    class AudioTeask implements Runnable {
        @Override
        public void run() {
            audioRecord.startRecording();
            //此处获取的音频数据是pcm音频数据，是原始数据。需要编码后再推到服务器
            byte[] bytes = new byte[inputSamples];
            while (isLiving) {
                int len = audioRecord.read(bytes, 0, bytes.length);
                mLivePusher.native_pushAudio(bytes);//声明一个native方法
            }
        }
    }

    public void stopLive() {
        isLiving = false;
    }
}