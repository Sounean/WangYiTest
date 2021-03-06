//
// Created by 86173 on 2019/7/29.
//

#ifndef WANGYIPUSH_VIDEOCHANNEL_H
#define WANGYIPUSH_VIDEOCHANNEL_H


#include "librtmp/rtmp.h"

class VideoChannel {

    typedef void (*VideoCallback)(RTMPPacket* packet);

public:
    void setVideoEncInfo(int width, int height, int fps, int bitrate);

    void encodeData(int8_t *data);
    void setVideoCallback(VideoCallback videoCallback);

private:
    int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;
    int ySize;
    int uvSize;
    x264_t *videoCodec;
    //x264编码后的一帧， 临时存储， 之后还要转化成NALU
    x264_picture_t *pic_in;
    VideoCallback videoCallback;
    void sendFrame(int type, uint8_t *payload, int i_payload);
    void sendSpsPps(uint8_t sps[100], uint8_t pps[100], int len, int pps_len);
};


#endif //WANGYIPUSH_VIDEOCHANNEL_H
