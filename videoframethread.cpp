#include "videoframethread.h"

VideoFrameThread::VideoFrameThread(AVFormatContext * ptr_format_ctx, AVCodecContext * ptr_codec_ctx, int video_index)
    : QThread(NULL)
{
    ptr_format_ctx_ = ptr_format_ctx;
    ptr_codec_ctx_ = ptr_codec_ctx;
    video_index_ = video_index;
    loop_quit_ = false;
    pause_ = false;
}

VideoFrameThread::~VideoFrameThread()
{
    loop_quit_ = true;
    this->quit();
    this->wait();
    qDebug("VideoFrameThread delete");
}

void VideoFrameThread::resume()
{
    pause_ = false;
}

void VideoFrameThread::pause()
{
    pause_ = true;
}


// virtual function
void VideoFrameThread::run()
{
    AVPacket packet;
    int ret;

    while(!loop_quit_) {
        if(pause_ == false) {
            ret = av_read_frame(ptr_format_ctx_, &packet);
            if(0 == ret) {
                if(packet.stream_index == video_index_) {
                    ret = avcodec_send_packet(ptr_codec_ctx_, &packet);
                    if(ret < 0) {
                        qDebug("avcodec_send_packet error %d\n", ret);
                        continue ;
                    }
                    av_packet_unref(&packet);

                    AVFrame * frame = av_frame_alloc();
                    ret = avcodec_receive_frame(ptr_codec_ctx_, frame);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        av_frame_free(&frame);
                        continue ;
                    } else if(ret < 0) {
                        qDebug("avcodec_receive_frame error %d\n", ret);
                        av_frame_free(&frame);
                        continue ;
                    }

                    emit frame_ready(frame, ptr_codec_ctx_->frame_number);
                }
            }
        } else {
            QThread::msleep(100);
        }
    }

    // event loop, wait user to call exit() or quit()
    this->exec();
}
