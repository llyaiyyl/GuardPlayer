#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QImage>
#include <QTimer>
#include <QMutex>

#include <queue>

#include "videoframethread.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavdevice/avdevice.h"
}

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    enum URL_TYPE {
        URL_TYPE_RTSP = 0,
        URL_TYPE_FILE,
        URL_TYPE_V4L2
    };

    explicit VideoWidget(QWidget *parent, const QRect &rect);
    ~VideoWidget();

    int32_t init(const char * url, enum VideoWidget::URL_TYPE url_type);
    int32_t play();

public slots:
    void frame_ready(AVFrame * ptr_frame, int frame_num);
    void on_timerout(void);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QTimer * timer_;
    QMutex mutex_;

    AVFormatContext *ptr_format_ctx_;
    AVCodecContext *ptr_codec_ctx_;
    AVCodec *ptr_codec_;
    AVPacket packet_;
    AVFrame *frame_;
    struct SwsContext *sws_ctx_;
    AVDictionary * opts_;
    uint8_t *dst_data_[4];
    int dst_linesize_[4];
    int videoStream_;
    int url_type_;
    int fps_;

    VideoFrameThread * vft_;

    std::queue<AVFrame *> queue_frame_;
};

#endif // VIDEOWIDGET_H
