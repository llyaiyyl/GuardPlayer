#ifndef GUARDPLAYER_H
#define GUARDPLAYER_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>

#include <list>

#include "videowidget.h"

class VideoWidget;

class GuardPlayer : public QWidget
{
    Q_OBJECT
public:
    GuardPlayer(QWidget *parent = 0);
    ~GuardPlayer();

private:
    std::list<VideoWidget *> list_vw_;
};

#endif // GUARDPLAYER_H


