#include "htrackbox.h"
#include "ui_htrackbox.h"
#include "hmainwindow.h"
#include "kfadewidgeteffect.h"

#include <QTimer>

QHash<QString, HTrackBox*> HTrackBox::s_map;

HTrackBox* HTrackBox::getBox(HTrack &rep) {
    QString dumbName=rep.getTrackName()+"__"+rep.getArtistName();
    if(s_map.contains(dumbName)) return s_map[dumbName];
    s_map[dumbName] = new HTrackBox(rep);
    return s_map[dumbName];
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

    s_ref.sendPlayCount(this,"setPlayCount");
    s_ref.sendUserPlayCount(this,"setUserPlayCount");
    s_ref.sendLoved(this,"setLoved");
    s_ref.sendTagNames(this,"setTags");
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
    KFadeWidgetEffect* kwe=new KFadeWidgetEffect(this);
    ui->label_plays->setText("<B>"+QString::number(s_playCountCache)+"</B> plays/<B>"+QString::number(s_userPlayCountCache)+"</B> by you");
    ui->label_trackName->setText("<B><A href=\"more\">"+s_ref.getTrackName()+(s_lovedCache?" &#x2665; ":" ")+"by "+s_ref.getArtistName()+"</B></A>");
    ui->label_tags->setText(s_tags);
    QTimer::singleShot(0,kwe,SLOT(start()));
}
