#include "hplayercontext.h"
#include "ui_hplayercontext.h"
#include "htrackcontext.h"
#include "hrdiointerface.h"

HPlayerContext* HPlayerContext::_singleton = 0;

HPlayerContext::HPlayerContext(QWidget *parent) :
    QWidget(parent),
    s_magic(0),
    ui(new Ui::HPlayerContext)
{
    ui->setupUi(this);
    ui->widget_trackContext->setLayout(new QVBoxLayout);
    connect(HRdioInterface::singleton(),SIGNAL(playingTrackChanged(HTrack&)),this,SLOT(showTrack(HTrack&)));
    showTrack(HTrack::get(HRdioInterface::singleton()->artistName(),HRdioInterface::singleton()->trackName()));
}

HPlayerContext::~HPlayerContext()
{
    delete ui;
}

void HPlayerContext::showTrack(HTrack &t) {
    ui->widget_trackContext->layout()->removeWidget(s_magic);
    delete s_magic;
    s_magic=new HTrackContext(t);
    ui->widget_trackContext->layout()->addWidget(s_magic);
}
