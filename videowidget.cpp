#include "videowidget.h"

VideoWidget::VideoWidget(QWidget *parent, const QRect &rect)
    : QWidget(parent)
{
    this->setGeometry(rect);

    ptr_format_ctx_ = NULL;
    ptr_codec_ctx_ = NULL;
    ptr_codec_ = NULL;
    frame_ = NULL;
    sws_ctx_ = NULL;
    opts_ = NULL;

    dst_data_[0] = NULL;

    vft_ = NULL;

    timer_ = NULL;
}

VideoWidget::~VideoWidget()
{
    if(timer_) {
        delete timer_;
        timer_ = NULL;
    }

    if(vft_) {
        delete vft_;
        vft_ = NULL;
    }

    if(dst_data_[0]) {
        av_freep(&dst_data_[0]);
        dst_data_[0] = NULL;
    }

    if(sws_ctx_) {
        sws_freeContext(sws_ctx_);
        sws_ctx_ = NULL;
    }

    if(ptr_codec_ctx_) {
        avcodec_close(ptr_codec_ctx_);
    }

    if(ptr_format_ctx_) {
        avformat_close_input(&ptr_format_ctx_);
    }
}

// public function
int32_t VideoWidget::init(const char * url, enum VideoWidget::URL_TYPE url_type)
{
    int ret;

    url_type_ = url_type;
    if(url_type_ == VideoWidget::URL_TYPE_RTSP) {
        av_dict_set(&opts_, "rtsp_transport", "tcp", 0);
        ret = avformat_open_input(&ptr_format_ctx_, url, NULL, &opts_);
        if(ret) {
            qDebug("avformat_open_input error %d\n", ret);
            return ret;
        }
    } else if(url_type_ == VideoWidget::URL_TYPE_FILE) {
        ret = avformat_open_input(&ptr_format_ctx_, url, NULL, NULL);
        if(ret) {
            qDebug("avformat_open_input error %d\n", ret);
            return ret;
        }
    } else if(url_type_ == VideoWidget::URL_TYPE_V4L2) {
        avdevice_register_all();
        ret = avformat_open_input(&ptr_format_ctx_, url, NULL, NULL);
        if(ret) {
            qDebug("avformat_open_input error %d\n", ret);
            return ret;
        }
    }
    else {
        return 1;
    }

    ret = avformat_find_stream_info(ptr_format_ctx_, NULL);
    if(ret < 0) {
        qDebug("avformat_find_stream_info error %d\n", ret);
        goto err0;
    }
    av_dump_format(ptr_format_ctx_, 0, url, 0);

    videoStream_ = -1;
    for(uint32_t i = 0; i < ptr_format_ctx_->nb_streams; i++) {
        if(ptr_format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream_ = i;
            break;
        }
    }
    if(videoStream_ == -1) {
        ret = videoStream_;
        goto err0;
    }
    fps_ = (ptr_format_ctx_->streams[videoStream_]->avg_frame_rate.num) * 1.0 / ptr_format_ctx_->streams[videoStream_]->avg_frame_rate.den;

    ptr_codec_ = avcodec_find_decoder(ptr_format_ctx_->streams[videoStream_]->codecpar->codec_id);
    if(NULL == ptr_codec_) {
        ret = -1;
        goto err0;
    }

    ptr_codec_ctx_ = avcodec_alloc_context3(ptr_codec_);
    avcodec_parameters_to_context(ptr_codec_ctx_, ptr_format_ctx_->streams[videoStream_]->codecpar);
    ret = avcodec_open2(ptr_codec_ctx_, ptr_codec_, NULL);
    if(ret < 0) {
        qDebug("avcodec_open2 error\n");
        goto err0;
    }
    qDebug("video src: %d x %d, pix_fmt: %d, fps: %d", ptr_codec_ctx_->width,
           ptr_codec_ctx_->height,
           ptr_codec_ctx_->pix_fmt,
           fps_);

    float ratio;
    uint32_t dst_w, dst_h;
    ratio = ptr_codec_ctx_->width / (this->width() * 1.0);
    dst_w = this->width();
    dst_h = ptr_codec_ctx_->height / ratio;
    this->resize(dst_w, dst_h);
    sws_ctx_ = sws_getContext(ptr_codec_ctx_->width,
                              ptr_codec_ctx_->height,
                              ptr_codec_ctx_->pix_fmt,
                              dst_w,
                              dst_h,
                              AV_PIX_FMT_RGB24,
                              SWS_BILINEAR,
                              NULL,
                              NULL,
                              NULL);
    if(NULL == sws_ctx_) {
        qDebug("sws_getContext error\n");
        ret = -1;
        goto err1;
    }
    qDebug("video to: %d x %d, pix_fmt: %d, fps: %d", this->width(),
           this->height(),
           ptr_codec_ctx_->pix_fmt,
           fps_);

    ret = av_image_alloc(dst_data_, dst_linesize_, this->width(), this->height(), AV_PIX_FMT_RGB24, 1);
    if(ret < 0) {
        qDebug("av_image_alloc error\n");
        goto err2;
    }
    return 0;

err2:
    sws_freeContext(sws_ctx_);
    sws_ctx_ = NULL;
err1:
    avcodec_close(ptr_codec_ctx_);
    ptr_codec_ctx_ = NULL;
err0:
    avformat_close_input(&ptr_format_ctx_);
    ptr_format_ctx_ = NULL;
    return ret;
}

int32_t VideoWidget::play()
{
    vft_ = new VideoFrameThread(ptr_format_ctx_, ptr_codec_ctx_, videoStream_);
    connect(vft_, SIGNAL(frame_ready(AVFrame*,int)), this, SLOT(frame_ready(AVFrame*,int)));
    vft_->start();


    if(url_type_ == VideoWidget::URL_TYPE_RTSP || url_type_ == VideoWidget::URL_TYPE_V4L2) {

    } else {
        timer_ = new QTimer(this);
        connect(timer_, SIGNAL(timeout()), this, SLOT(on_timerout()));
        timer_->start(1000.0 / fps_);
    }

    return 0;
}

// slots
void VideoWidget::frame_ready(AVFrame *ptr_frame, int frame_num)
{
    frame_num = frame_num;

    if(url_type_ == VideoWidget::URL_TYPE_RTSP || url_type_ == VideoWidget::URL_TYPE_V4L2) {
        sws_scale(sws_ctx_,
                  (const uint8_t * const *)ptr_frame->data,
                  ptr_frame->linesize,
                  0,
                  ptr_frame->height,
                  dst_data_,
                  dst_linesize_);

        this->repaint();
        av_frame_free(&ptr_frame);
    } else {
        mutex_.lock();
        queue_frame_.push(ptr_frame);
        mutex_.unlock();
    }
}

void VideoWidget::on_timerout()
{
    mutex_.lock();

    if(queue_frame_.size() >= 200) {
        vft_->pause();
        // qDebug("thread pause");
    } else if(queue_frame_.size() <= 100) {
        vft_->resume();
        // qDebug("thread resume");
    }

    if(queue_frame_.size()) {
        AVFrame * ptr_frame;
        ptr_frame = queue_frame_.front();

        sws_scale(sws_ctx_,
                  (const uint8_t * const *)ptr_frame->data,
                  ptr_frame->linesize,
                  0,
                  ptr_frame->height,
                  dst_data_,
                  dst_linesize_);
        this->repaint();
        av_frame_free(&ptr_frame);

        queue_frame_.pop();
    }

    mutex_.unlock();
}

// virtual function
void VideoWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(event->rect(), QImage(dst_data_[0], this->width(), this->height(), QImage::Format_RGB888));
}
















