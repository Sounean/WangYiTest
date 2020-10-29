#include <jni.h>
#include <string>
#include <pthread.h>
#include "x264.h"
#include "librtmp/rtmp.h"
#include "include/x264.h"
#include "VideoChannel.h"
#include "macro.h"
#include "safe_queue.h"
#include "AudioChannel.h"

/*
 * 开了个线程，不断地去推流到服务器。
 * */

VideoChannel *videoChannel;//编码专用，会回调编码之后的RTMPPacket
AudioChannel *audioChannel;
int isStart = 0;//为了防止用户重复点击开始直播，导致重新初始化
pthread_t pid; //连接服务器的线程
uint32_t start_time;//开始推流时间戳（做音视频同步用）
int readyPushing = 0;//通过该标志位来判断 是否已连接服务器，准备就绪
SafeQueue<RTMPPacket *> packets;//队列，用于存储VideoChannel中组装好准备传输的RTMPPacket (主要的就put和get方法)。


/**
 * VideoChannel的回调方法，会收到VideoChannel中编码之后的每个RTMPPacket，计入队列等待推流（上传服务器）
 * @param packet
 */
void callback(RTMPPacket *packet) {
    if (packet) {
        //设置时间戳
        packet->m_nTimeStamp = RTMP_GetTime() - start_time;
        //加入队列
        packets.put(packet);
    }
}

/**
 * 释放packet
 * @param packet
 */
void releasePackets(RTMPPacket *&packet) {
    if (packet) {//如果不为空，就释放掉
        RTMPPacket_Free(packet);
        delete packet;
        packet = 0;
    }

}

/**
 * 开始推流
 * 该方法在开始直播的方法(Java_com_yu_mypush_LivePusher_native_1start)中调用，可以理解为Java里new Thread中的run()方法
 * @param args
 * @return
 */
void *start(void *args) {

    char *url = static_cast<char *>(args);
    RTMP *rtmp = 0;
    rtmp = RTMP_Alloc();//新建一个rtmp对象。
    if (!rtmp) {//如果实例化失败
        LOGE("alloc rtmp失败");
        return NULL;
    }

    /*
     * 实例化rtmp
     * */
    RTMP_Init(rtmp);
    int ret = RTMP_SetupURL(rtmp, url);
    if (!ret) {
        LOGE("设置地址失败:%s", url);
        return NULL;
    }
    rtmp->Link.timeout = 5;//设置连接时间。
    RTMP_EnableWrite(rtmp);
    ret = RTMP_Connect(rtmp, 0);//连接
    if (!ret) {
        LOGE("连接服务器:%s", url);
        return NULL;
    }
    ret = RTMP_ConnectStream(rtmp, 0);
    if (!ret) {
        LOGE("连接流:%s", url);
        return NULL;
    }
    start_time= RTMP_GetTime();//RTMP_GetTime()为获取推流时间。
    //表示可以开始推流了
    readyPushing = 1;
    packets.setWork(1);
    RTMPPacket *packet = 0;
    while (readyPushing) {
//        队列取数据  pakets
        packets.get(packet);
        LOGE("取出一帧数据");
        if (!readyPushing) {
            break;
        }
        if (!packet) {
            continue;
        }
        packet->m_nInfoField2 = rtmp->m_stream_id;
        ret = RTMP_SendPacket(rtmp, packet, 1);
//        packet 释放
        releasePackets(packet);
    }
//  用户结束直播后，开始把标志位都初始化掉。
    isStart = 0;
    readyPushing = 0;
    packets.setWork(0);//packet不工作
    packets.clear();//packet清空
    if (rtmp) {//关闭链接，释放rtmp对象。
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    delete (url);
    return  0;
}







extern "C" JNIEXPORT jstring JNICALL
Java_com_example_wangyipush_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    x264_picture_t* x264_picture = new x264_picture_t;  // 新建一个x264的图片。
    RTMP_Alloc();
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
/*
 * 初始化
 * */
Java_com_example_wangyipush_LivePusher_native_1init(JNIEnv *env, jobject instance) {
    videoChannel = new VideoChannel;
    //设置回调，因为VideoChannel只负责编码，这里拿到编码之后的数据进行传输
    videoChannel->setVideoCallback(callback);

    audioChannel = new AudioChannel;
    audioChannel->setAudioCallback(callback);
}

extern "C"
JNIEXPORT void JNICALL
/**
 * 初始化视频数据，在摄像头数据尺寸变化时Java层调用的（第一次打开摄像头、摄像头切换、横竖屏切换都会引起摄像头采集的尺寸发送变化，会走这个方法）
 * @param env
 * @param instance
 * @param width
 * @param height
 * @param fps
 * @param bitrate
 */
Java_com_example_wangyipush_LivePusher_native_1setVideoEncInfo(JNIEnv *env, jobject instance,
                                                               jint width, jint height, jint fps,
                                                               jint bitrate) {

    if (!videoChannel) {    // 如果videoChannel不为空。
        return;
    }
    videoChannel->setVideoEncInfo(width, height, fps, bitrate);

}


/*
 * 开始直播
 * */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_wangyipush_LivePusher_native_1start(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    if (isStart) {
        return;
    }
    isStart = 1;

    // 因为path会回收，销毁；所以需要一个临时变量用来保存path。
    char *url = new char[strlen(path) + 1];
    strcpy(url, path);

    //start类似java线程中的run方法，url是start的参数
    pthread_create(&pid, 0, start, url);//url地址会被当作参数传入上文定义的start函数中去。

    env->ReleaseStringUTFChars(path_, path);
}


/*
 * 开始数据编码（开始后会收到每个编码后的packet回调并加入到队列中）
 * 该方法会在开始直播并且收到摄像头返回数据之后调用
 * @param env
 * @param instance
 * @param data_
 * */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_wangyipush_LivePusher_native_1pushVideo(JNIEnv *env, jobject instance,
                                                         jbyteArray data_) {
    jbyte *data = env->GetByteArrayElements(data_, NULL);

    if (!videoChannel || !readyPushing) {
        return;
    }
    videoChannel->encodeData(data);

    env->ReleaseByteArrayElements(data_, data, 0);
}



/*
 * 音频推流部分
 * */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_wangyipush_LivePusher_native_1pushAudio(JNIEnv *env, jobject instance,jbyteArray bytes_) {
    jbyte *bytes = env->GetByteArrayElements(bytes_, NULL);
    //pcm
    jbyte *data = env->GetByteArrayElements(bytes_, NULL);
    // 我们先判断当前的Audio是否为空.如果不为空就继续往下走，如果为空表示没准备好则return
    if (!audioChannel || !readyPushing) {
        return;
    }
    audioChannel->encodeData(data);
    env->ReleaseByteArrayElements(bytes_, data, 0);

    env->ReleaseByteArrayElements(bytes_, bytes, 0);
}



/*
 * 由java层传递过来的参数
 * */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_wangyipush_LivePusher_native_1setAudioEncInfo(JNIEnv *env, jobject instance,
                                                               jint sampleRateInHz, jint channels) {

    if (audioChannel) {
        audioChannel->setAudioEncInfo(sampleRateInHz, channels);
    }

}


extern "C"
JNIEXPORT jint JNICALL
Java_com_example_wangyipush_LivePusher_getInputSamples(JNIEnv *env, jobject instance) {

    if (audioChannel) {
        return audioChannel->getInputSamples();
    }
    return -1;

}