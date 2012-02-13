#include "hplayercontext.h"
#include "ui_hplayercontext.h"
#include "htrackcontext.h"
#include "hslideshow.h"
#include "habstractmusicinterface.h"

HPlayerContext* HPlayerContext::_singleton = 0;

HPlayerContext::HPlayerContext(QWidget *parent) :
    QWidget(parent),
    s_magic(0),
    s_slideshow(0),
    ui(new Ui::HPlayerContext)
{
    ui->setupUi(this);
    ui->widget_trackContext->setLayout(new QVBoxLayout);
    connect(HPlayer::singleton(),SIGNAL(trackChanged(HTrack&)),this,SLOT(showTrack(HTrack&)));
    if(HPlayer::singleton()->currentTrack()) showTrack(*HPlayer::singleton()->currentTrack());
}

HPlayerContext::~HPlayerContext()
{
    delete ui;
}

void HPlayerContext::showEvent(QShowEvent *e) {
    if(s_magic) {
        showTrack(s_magic->getTrack());
    }
    QWidget::showEvent(e);
}

void HPlayerContext::showTrack(HTrack &t) {
    qDebug()<<"ST::"<<t.getTrackName();
    if(s_magic) {
        ui->widget_trackContext->layout()->removeWidget(s_magic);
        s_magic->setSlideshow(0);
        s_magic=0;
    }
    s_magic=HTrackContext::getContext(t);
    if(!isVisible()) return;
    s_magic->setParent(0);
    ui->widget_trackContext->layout()->addWidget(s_magic);

    s_magic->show();
    s_slideshow=HSlideshow::getSlideshow(t.getArtist());
    s_magic->setSlideshow(s_slideshow);
    s_slideshow->adjustSize();
    qDebug()<<"ST::END";
}
