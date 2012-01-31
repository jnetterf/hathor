#include "hplayercontext.h"
#include "ui_hplayercontext.h"
#include "htrackcontext.h"
#include "hrdioprovider.h"
#include "hslideshow.h"

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

void HPlayerContext::showTrack(HTrack &t) {
//    delete s_magic;
    if(s_magic) {
        s_magic->hide();
//        s_magic->deleteLater();
    }
    s_magic=new HTrackContext(t);
    ui->widget_trackContext->layout()->addWidget(s_magic);

    if(s_slideshow) s_slideshow->pause();
    s_slideshow=HSlideshow::getSlideshow(t);
    s_magic->setSlideshow(s_slideshow);
    s_slideshow->adjustSize();
}
