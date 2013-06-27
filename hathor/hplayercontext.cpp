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
    ui->widget_trackContext->layout()->setContentsMargins(0,0,0,0);
    connect(HPlayer::singleton(),SIGNAL(trackChanged(HTrack&)),this,SLOT(showTrack(HTrack&)));
    if(HPlayer::singleton()->currentTrack()) showTrack(*HPlayer::singleton()->currentTrack());
}

HPlayerContext::~HPlayerContext()
{
    delete s_magic;
    delete ui;
}

void HPlayerContext::showEvent(QShowEvent *e) {
    if(s_magic) {
        showTrack(s_magic->getTrack());
    }
    QWidget::showEvent(e);
}

void HPlayerContext::hideEvent(QHideEvent*) {
    if(s_magic) s_magic->setSlideshow(0);
    delete s_slideshow;
    s_slideshow=0;
}

void HPlayerContext::showTrack(HTrack &t) {
    if(s_magic) {
        ui->widget_trackContext->layout()->removeWidget(s_magic);
        s_magic->setSlideshow(0);
        if((t.getTrackName()==s_magic->getTrack().getTrackName())&&(t.getArtistName()==s_magic->getTrack().getArtistName())) {
            if(s_slideshow) s_slideshow->setParent(0);
        }
        delete s_magic;
        s_magic=0;
    }
    s_magic=new HTrackContext(t);
    if(!isVisible()) return;
    s_magic->setParent(0);
    ui->widget_trackContext->layout()->addWidget(s_magic);

    s_magic->show();
    // Disable this until I find the mem leak.
    s_slideshow=HSlideshow::getSlideshow(t.getArtist());
    s_magic->setSlideshow(s_slideshow);
    s_slideshow->adjustSize();
}
