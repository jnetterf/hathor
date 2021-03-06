#include "halbumbox.h"
#include "ui_halbumbox.h"
#include "hloginwidget.h"
#include "hmainwindow.h"
#include "kfadewidgeteffect.h"

HAlbumBox* HAlbumBox::getBox(HAlbum &rep) {
    return new HAlbumBox(rep);
}

HAlbumBox::HAlbumBox(HAlbum &album, QWidget *parent) :
    HGrowingWidget(parent),
    s_album(album),
    s_cachedPlayCount(-1),s_cachedListenerCount(-1),s_cachedUserPlayCount(-1),
    s_gotTags(0),
    ui(new Ui::HAlbumBox)
{
    ui->setupUi(this);

    connect(ui->label_title,SIGNAL(linkActivated(QString)),this,SLOT(requestContext()));
    connect(this,SIGNAL(contextRequested(HAlbum&)),HMainWindow::singleton(),SLOT(showContext(HAlbum&)));

    s_priority[1].push_back(album.sendTagNames(this,"setTagNames"));
    ui->label_title->setText("<B><A href=\"more\">"+album.getAlbumName()+"</A></B>");
    ui->label_artist->setText("by <B>"+album.getArtistName()+"</B>");
    s_priority[1].push_back(album.sendPlayCount(this,"setPlayCount"));
    s_priority[1].push_back(album.sendListenerCount(this,"setListenerCount"));
    s_priority[1].push_back(album.sendUserPlayCount(this,"setUserPlayCount"));
    readjustPriorities();
}

void HAlbumBox::showEvent(QShowEvent *e) { s_showTime=QTime::currentTime(); s_priority[0].push_back(s_album.sendPic(HAlbum::Large,this,"setPixmap")); readjustPriorities(); QWidget::showEvent(e); }
void HAlbumBox::hideEvent(QHideEvent *e) { readjustPriorities(); QWidget::hideEvent(e); }

HAlbumBox::~HAlbumBox()
{
    delete ui;
}


void HAlbumBox::setPlayCount(int count) {
    s_cachedPlayCount=count;
    updateCounts();
}

void HAlbumBox::setListenerCount(int count) {
    s_cachedListenerCount=count;
    updateCounts();
}

void HAlbumBox::setUserPlayCount(int count) {
    s_cachedUserPlayCount=count;
    updateCounts();
}

void HAlbumBox::updateCounts() {
    if(!s_gotTags||(s_cachedPlayCount==-1)||(s_cachedListenerCount==-1)||(s_cachedUserPlayCount==-1)) return;
    KFadeWidgetEffect* kwe=0;
    if(s_showTime.msecsTo(QTime::currentTime())>110) {
        kwe=new KFadeWidgetEffect(this);
    }
    ui->label_summary->setText("<B>"+QString::number(s_cachedPlayCount)+"</B> plays by "+QString::number(s_cachedListenerCount)+" listeners<br><B>"+QString::number(s_cachedUserPlayCount)+"</B> plays in your library");
    ui->label_summary->adjustSize();
    ui->label_tags->setText(s_tagString);
    ui->label_tags->adjustSize();
    if(kwe) QTimer::singleShot(10,kwe,SLOT(start()));
}


void HAlbumBox::setPixmap(QImage& p) {
    if(!isVisible()) return;
    if(p.width()!=174) p=p.scaledToWidth(174,Qt::SmoothTransformation);
    ui->label_icon->setPixmap(p);
    ui->label_icon->adjustSize();
    adjustSize();
}

void HAlbumBox::setTagNames(QStringList tsl) {
    s_gotTags=1;
    for(int i=4;i<tsl.size();i++) {
        tsl.removeAt(i);
    }
    s_tagString=tsl.join(", ");
    updateCounts();
}
