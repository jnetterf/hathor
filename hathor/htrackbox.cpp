#include "htrackbox.h"
#include "ui_htrackbox.h"
#include "hmainwindow.h"
#include "kfadewidgeteffect.h"

#include <QTimer>

HTrackBox* HTrackBox::getBox(HTrack &rep) {
    return new HTrackBox(rep);
}

HTrackBox::HTrackBox(HTrack &ref, QWidget *parent) :
    HGrowingWidget(parent),
    s_ref(ref),
    s_lovedCache(0),
    s_score(0),
    ui(new Ui::HTrackBox)
{
    ui->setupUi(this);

    ui->label_trackName->setText("<B><A href=\"more\">"+s_ref.getTrackName()+" by "+s_ref.getArtistName()+"</B></A>");
    connect(ui->label_trackName,SIGNAL(linkActivated(QString)),this,SLOT(requestContext()));
    connect(this,SIGNAL(contextRequested(HTrack&)),HMainWindow::singleton(),SLOT(showContext(HTrack&)));

    s_priority[0].push_back(s_ref.sendPlayCount(this,"setPlayCount"));
    s_priority[0].push_back(s_ref.sendUserPlayCount(this,"setUserPlayCount"));
    s_priority[0].push_back(s_ref.sendLoved(this,"setLoved"));
    s_priority[1].push_back(s_ref.sendTagNames(this,"setTags"));
    readjustPriorities();
}

HTrackBox::~HTrackBox()
{
    delete ui;
}

void HTrackBox::setLoved(bool a) {
    s_lovedCache=a;
    if(++s_score==4) updateBoxes();
}

void HTrackBox::setPlayCount(int a) {
    s_playCountCache=a;
    if(++s_score==4) updateBoxes();
}

void HTrackBox::setUserPlayCount(int a) {
    s_userPlayCountCache=a;
    if(++s_score==4) updateBoxes();
}

void HTrackBox::setTags(QStringList tsl) {
    for(int i=4;i<tsl.size();i++) {
        tsl.removeAt(i);
    }
    s_tags=tsl.join(", ");
    if(++s_score==4) updateBoxes();
}

void HTrackBox::updateBoxes() {
    KFadeWidgetEffect* kwe=0;
    if(isVisible()) {
        kwe=new KFadeWidgetEffect(this);
    }
    ui->label_plays->setText("<B>"+QString::number(s_playCountCache)+"</B> plays/<B>"+QString::number(s_userPlayCountCache)+"</B> by you");
    ui->label_trackName->setText("<B><A href=\"more\">"+s_ref.getTrackName()+(s_lovedCache?" &#x2665; ":" ")+"by "+s_ref.getArtistName()+"</B></A>");
    ui->label_tags->setText(s_tags);
    if(kwe) {
        QTimer::singleShot(0,kwe,SLOT(start()));
    }
}
