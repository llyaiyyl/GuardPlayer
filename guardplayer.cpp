#include "guardplayer.h"

GuardPlayer::GuardPlayer(QWidget *parent)
    : QWidget(parent)
{
    this->setFixedSize(800, 600);

    VideoWidget * vw;
    int ret;

    vw = new VideoWidget(this, QRect(10, 10, 780, 780));
    // ret = vw->init("/dev/video0", VideoWidget::URL_TYPE_V4L2);
    ret = vw->init("../../Videos/threebody.mp4", VideoWidget::URL_TYPE_FILE);
    if(ret) {
        qDebug("VideoWidget init error");
        delete vw;
    } else {
        vw->play();
        list_vw_.push_back(vw);
    }
}

GuardPlayer::~GuardPlayer()
{
    if(list_vw_.size()) {
        std::list<VideoWidget *>::iterator it;
        for(it = list_vw_.begin(); it != list_vw_.end(); it++) {
            delete (*it);
        }

        list_vw_.clear();
    }
}











