//
// Created by 86173 on 2019/8/8.
//

#ifndef WANGYIPUSH_AUDIOCHANNEL_H
#define WANGYIPUSH_AUDIOCHANNEL_H


#include <faac.h>
#include <jni.h>
#include <pty.h>
#include "librtmp/rtmp.h"

class AudioChannel {
    typedef void (*AudioCallback)(RTMPPacket *packet);//把处理好的音频数据放到队列中去的回调方法。因为要回调，所以设置一个接口
public:
    void encodeData(int8_t *data);
    void setAudioEncInfo(int samplesInHZ, int channels);


    jint getInputSamples();
    ~AudioChannel();

    void setAudioCallback(AudioCallback audioCallback);

private://下面很多编码器的参数
    AudioCallback audioCallback;
    int mChannels;
    faacEncHandle audioCodec;//一个编码器参数
    u_long inputSamples;//缓存区大小
    u_long maxOutputBytes;//最大缓存区大小
    u_char *buffer = 0;
};


#endif //WANGYIPUSH_AUDIOCHANNEL_H
