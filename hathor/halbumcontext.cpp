#include "halbumcontext.h"
#include "ui_halbumcontext.h"
#include "halbumbox.h"
#include "htrackbox.h"
#include "htagbox.h"
#include "halbumbox.h"
#include "hshoutbox.h"
#include "hartistbox.h"
#include "habstractmusicinterface.h"
#include <QRect>
#include <QScrollBar>
#include <QMenu>
#include "kfadewidgeteffect.h"

QHash<QString, HAlbumContext*> HAlbumContext::s_map;

HAlbumContext* HAlbumContext::getContext(HAlbum &rep) {
    QString dumbName=rep.getAlbumName()+"__"+rep.getArtistName();
    if(s_map.contains(dumbName)) return s_map[dumbName];
    s_map[dumbName] = new HAlbumContext(rep);
    return s_map[dumbName];
}

HAlbumContext::HAlbumContext(HAlbum& rep, QWidget *parent) :
    QWidget(parent),
    s_rep(rep),
    s_albumLoadCount(0),
    s_trackLoadCount(0),
    s_tagLoadCount(0),
    s_artistLoadCount(0),
    s_shoutLoadCount(0),
    s_cachedPlayCount(-1),s_cachedListenerCount(-1),s_cachedUserPlayCount(-1),
    ui(new Ui::HAlbumContext)
{
    s_showTime=QTime::currentTime();
    ui->setupUi(this);
    ui->label_album->setText(s_rep.getAlbumName());
    s_rep.sendPic(HAlbum::Large,this,"setPic");
    s_rep.sendSummary(this,"setSummary");

    s_rep.sendPlayCount(this,"setPlayCount");
    s_rep.sendListenerCount(this,"setListenerCount");
    s_rep.sendUserPlayCount(this,"setUserPlayCount");

    connect(ui->label_moreDescription,SIGNAL(linkActivated(QString)),this,SLOT(showMoreBio()));

    ui->widget_artist->setLayout(new QVBoxLayout);
    ui->widget_tags->setLayout(new QVBoxLayout);
    ui->widget_tracks->setLayout(new QVBoxLayout);
    ui->widget_comments->setLayout(new QVBoxLayout);
    ui->widget_comments->layout()->setSpacing(0);

    loadShouts();

    connect(ui->label_moreTracks,SIGNAL(linkActivated(QString)),this,SLOT(loadTracks()));

    connect(ui->label_moreTags,SIGNAL(linkActivated(QString)),this,SLOT(loadTags()));

//    connect(ui->label_moreArtists,SIGNAL(linkActivated(QString)),this,SLOT(loadSimilar()));

    connect(ui->label_moreShoutbox,SIGNAL(linkActivated(QString)),this,SLOT(loadShouts()));

    ui->label_you->setPixmap(HUser::get(lastfm::ws::Username).getPic(HUser::Medium).scaledToWidth(70,Qt::SmoothTransformation));

    ui->label_albumPic->adjustSize();
    ui->frame_art->adjustSize();
    ui->frame_header->adjustSize();

    connect(ui->button_play,SIGNAL(clicked()),this,SLOT(play()));
}

HAlbumContext::~HAlbumContext()
{
    delete ui;
}

void HAlbumContext::showEvent(QShowEvent * e)
{
    s_showTime=QTime::currentTime();
    //our boxes may have been stolen while we weren't looking >_<

    while(ui->widget_artist->layout()->count()) {
        ui->widget_artist->layout()->removeItem(ui->widget_artist->layout()->itemAt(0));
    }
    while(ui->widget_tags->layout()->count()) {
        ui->widget_tags->layout()->removeItem(ui->widget_tags->layout()->itemAt(0));
    }
    while(ui->widget_tracks->layout()->count()) {
        ui->widget_tracks->layout()->removeItem(ui->widget_tracks->layout()->itemAt(0));
    }


    s_albumLoadCount=0;
    s_trackLoadCount=0;
    s_tagLoadCount=0;

    loadArtist();
    loadTracks();
    loadTags();

    QWidget::showEvent(e);
}

void HAlbumContext::showMoreBio()
{
    ui->label_moreDescription->setText("Loading...");
    deltaWidth=-ui->label_albumPic->pixmap()->width();
    int deltaHeight=-ui->label_albumPic->pixmap()->height();
    deltaWidth+=ui->label_albumPic->pixmap()->width();
    deltaHeight+=ui->label_albumPic->pixmap()->height();
    QPropertyAnimation* pa1=new QPropertyAnimation(ui->label_albumPic,"maximumSize");
    pa1->setStartValue(QSize(ui->label_albumPic->pixmap()->width()-deltaWidth,ui->label_albumPic->pixmap()->height()-deltaHeight));
    pa1->setEndValue(QSize(ui->label_albumPic->pixmap()->width(),ui->label_albumPic->pixmap()->height()));
    pa1->setDuration(300);
    pa1->start(QPropertyAnimation::DeleteWhenStopped);
    ui->label_albumPic->adjustSize();
    ui->frame_header->adjustSize();
    ui->label_description->adjustSize();
    s_rep.sendContent(this,"setSummary");
    ui->label_description->adjustSize();

    ui->label_moreDescription->hide();
}

void HAlbumContext::loadArtist()
{
    ui->label_moreArtists->setText("<p align=\"right\"><i>Loading...</i></p>");
    {
        HArtistBox* ab=HArtistBox::getBox(s_rep.getArtist());
        if(s_showTime.msecsTo(QTime::currentTime())>110||!s_rep.isCached()) {
            QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
            pa->setStartValue(0);
            pa->setEndValue(ab->sizeHint().height());
            pa->setDuration(500);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }
        ui->widget_artist->layout()->addWidget(ab);
    }
    s_artistLoadCount=1;
    ui->label_moreArtists->hide();
}

void HAlbumContext::loadTracks()
{
    ui->label_moreTracks->setText("<p align=\"right\"><i>Loading...</i></p>");
    s_rep.sendTracks(this,"loadTracks_2");
}

void HAlbumContext::loadTracks_2(QList<HTrack *>tracks)
{
    int i;
    int toLoad=s_trackLoadCount?s_trackLoadCount*2:10;
    for(i=s_trackLoadCount;i<tracks.size()&&i-s_trackLoadCount<toLoad;i++) {
        HTrackBox* ab=HTrackBox::getBox(*tracks[i]);
        if(s_showTime.msecsTo(QTime::currentTime())>110||!tracks[i]->isCached()) {
            QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
            pa->setStartValue(0);
            pa->setEndValue(32);
            pa->setDuration(500);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }
        ui->widget_tracks->layout()->addWidget(ab);
    }
    if(i-s_trackLoadCount!=toLoad) {
        ui->label_moreTracks->hide();
    } else {
        ui->label_moreTracks->setText(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
    }
    s_trackLoadCount=i;
}

void HAlbumContext::loadTags()
{
    ui->label_moreTags->setText("<p align=\"right\"><i>Loading...</i></p>");
    if (s_tagLoadCount) s_rep.sendMoreTags(this,"addTags");
    else s_rep.sendTags(this, "addTags");
}

void HAlbumContext::loadShouts()
{
    ui->label_moreShoutbox->setText("<p align=\"right\"><i>Loading...</i></p>");
    s_rep.sendShouts(this,"setShouts");
}

void HAlbumContext::play() {
    HPlayer::singleton()->clear();
    s_rep.sendTracks(HPlayer::singleton()->getStandardQueue(), "queue");
}

void HAlbumContext::setPlayCount(int a) {
    s_cachedPlayCount=a;
    updateBoxes();
}

void HAlbumContext::setListenerCount(int a) {
    s_cachedListenerCount=a;
    updateBoxes();
}

void HAlbumContext::setUserPlayCount(int a) {
    s_cachedUserPlayCount=a;
    updateBoxes();
}

void HAlbumContext::updateBoxes() {
    ui->label_playcount->setText("<B>"+QString::number(s_cachedPlayCount)+"</B> plays by <B>"+QString::number(s_cachedListenerCount)+"</B> users");
    ui->label_userplaycount->setText("<B>"+QString::number(s_cachedUserPlayCount)+"</B> plays in your library");
}

void HAlbumContext::setPic(QPixmap p) {
    ui->label_albumPic->setPixmap(p);
}

void HAlbumContext::setSummary(QString s) {
    ui->label_description->setFixedHeight(ui->label_description->height());
    ui->label_description->setText(s.size()?s:"No article is available for this album.");
    {
        QPropertyAnimation* pa=new QPropertyAnimation(ui->label_description,"maximumHeight");
        pa->setStartValue(ui->label_description->height());
        pa->setEndValue(ui->label_description->heightForWidth(ui->label_description->width()-deltaWidth));
        pa->setDuration((ui->label_description->heightForWidth(ui->label_description->width()-deltaWidth)-ui->label_description->height())*2);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void HAlbumContext::setShouts(QList<HShout *> shouts) {
    int i;
    int toLoad=s_shoutLoadCount?s_shoutLoadCount*2:10;
    for(i=s_shoutLoadCount;i<shouts.size()&&i-s_shoutLoadCount<toLoad;i++) {
        shouts[i]->getShouter().getPic(HUser::Medium);    //CACHE
        HShoutBox* ab=new HShoutBox(*shouts[i],this);
        ui->widget_comments->layout()->addWidget(ab);

    }
    if(i-s_shoutLoadCount!=toLoad) {
        ui->label_moreShoutbox->hide();
    } else {
        ui->label_moreShoutbox->setText(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
    }
    s_shoutLoadCount=i;
}

void HAlbumContext::addTags(QList<HTag*> tags) {
    int i;
    int toLoad=s_tagLoadCount?s_tagLoadCount*2:4;
    for(i=s_tagLoadCount;i<tags.size()&&i-s_tagLoadCount<toLoad;i++) {
        tags[i]->getContent();    //CACHE
        HTagBox* ab=HTagBox::getBox(*tags[i]);
        if(s_showTime.msecsTo(QTime::currentTime())>110) {
            QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
            pa->setStartValue(0);
            pa->setEndValue(40);
            pa->setDuration(500);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }
        ui->widget_tags->layout()->addWidget(ab);
    }
    if(i-s_tagLoadCount!=toLoad) {
        ui->label_moreTags->hide();
    } else {
        ui->label_moreTags->setText(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
    }
    s_tagLoadCount=i;
}
