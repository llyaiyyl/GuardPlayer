#ifndef VIDEOFRAMETHREAD_H
#define VIDEOFRAMETHREAD_H

#include <QWidget>
#include <QThread>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

class VideoFrameThread : public QThread
{
    Q_OBJECT
public:
    VideoFrameThread(AVFormatContext * ptr_format_ctx, AVCodecContext * ptr_codec_ctx, int video_index);
    ~VideoFrameThread();

    void resume(void);
    void pause(void);

protected:
    void run() override;

signals:
    void frame_ready(AVFrame * ptr_frame, int frame_num);

private:
    AVFormatContext *ptr_format_ctx_;
    AVCodecContext *ptr_codec_ctx_;
    int video_index_;
    bool loop_quit_;
    bool pause_;
};

#endif // VIDEOFRAMETHREAD_H
