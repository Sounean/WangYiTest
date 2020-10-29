//
// Created by 86173 on 2019/8/8.
//

#include <pty.h>
#include <cstring>
#include "AudioChannel.h"
#include "faac.h"
#include "include/faac.h"
#include "librtmp/rtmp.h"
#include "macro.h"

void AudioChannel::encodeData(int8_t *data) {//这个方法是不断的收到数据然后进行推流
    int bytelen= faacEncEncode(audioCodec, reinterpret_cast<int32_t *>(data), inputSamples, buffer, maxOutputBytes);//faac编码（编码器，数据，缓冲区大小，数据放在buffer这个位子，）
    // 对数据进行处理
    if (bytelen > 0) {//如果数据>0，证明有数据就开始
        RTMPPacket *packet = new RTMPPacket;//把数据的一帧打包成packet
        int bodySize = 2 + bytelen;
        RTMPPacket_Alloc(packet, bodySize);
        packet->m_body[0] = 0xAF;//第一个字节
        if (mChannels == 1) {
            packet->m_body[0] = 0xAE;
        }
        packet->m_body[1] = 0x01;//第二个字节，编码出来的声音都是0*01

        memcpy(&packet->m_body[2], buffer, bytelen);//编码之后aac的数据内容，这内容是不固定的，根据编码大小而得到的。把buffer数据放到packet去。
        //对帧数据进行赋值
        packet->m_hasAbsTimestamp = 0;//0代表相对时间
        packet->m_nBodySize = bodySize;//数据包大小
        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;//数据包类型（音频数据）
        packet->m_nChannel = 0x11;//频道
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
        audioCallback(packet);//把音频放到队列中去（队列只存在native-lib中）
    }

}


void AudioChannel::setAudioCallback(AudioChannel::AudioCallback audioCallback) {//对于接口方法的实现
    this->audioCallback = audioCallback;//给回调给native层
}

void AudioChannel::setAudioEncInfo(int samplesInHZ, int channels){// 调用前进行初始化faac编码器
    audioCodec=faacEncOpen(samplesInHZ, channels, &inputSamples, &maxOutputBytes);// 1.打开编码器,参数：频率、通道数、音频编码器提供的最小缓冲区大小（它是会比实际读到的缓冲区大一些）、最大缓冲区大小。
    //2.得到配置信息类，拿来设置参数
    faacEncConfigurationPtr config=faacEncGetCurrentConfiguration(audioCodec);
    config->mpegVersion = MPEG4;
    config->aacObjectType = LOW;    // lc标准
    config->inputFormat = FAAC_INPUT_16BIT; // 16位
    config->outputFormat = 0;
    //3.把配置信息放回编码器中。令其生效。
    faacEncSetConfiguration(audioCodec, config);
    buffer = new u_char[maxOutputBytes];
}

int AudioChannel::getInputSamples() {
    return inputSamples;
}

AudioChannel::~AudioChannel() {
    DELETE(buffer);
    //释放编码器
    if (audioCodec) {
        faacEncClose(audioCodec);
        audioCodec = 0;
    }
}
