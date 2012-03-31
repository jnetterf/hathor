#include "halbumcontext.h"
#include "ui_halbumcontext.h"
#include "halbumbox.h"
#include "htrackbox.h"
#include "htagbox.h"
#include "hlfmwebloginaction.h"
#include "halbumbox.h"
#include "hshoutbox.h"
#include "hartistbox.h"
#include "habstractmusicinterface.h"
#include "hnettloger.h"
#include <QRect>
#include <QScrollBar>
#include <QMenu>
#include "kfadewidgeteffect.h"

HAlbumContext* HAlbumContext::getContext(HAlbum &rep) {
    return new HAlbumContext(rep);
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
    s_shoutsToLoad(5),
    s_pw(0), s_ge(0),
    ui(new Ui::HAlbumContext)
{
    s_showTime=QTime::currentTime();
    ui->setupUi(this);
    ui->label_album->setText(s_rep.getAlbumName());
    s_priority[0].push_back(s_rep.sendSummary(this,"setSummary"));

    s_priority[0].push_back(s_rep.sendPlayCount(this,"setPlayCount"));
    s_priority[0].push_back(s_rep.sendListenerCount(this,"setListenerCount"));
    s_priority[0].push_back(s_rep.sendUserPlayCount(this,"setUserPlayCount"));

    connect(ui->label_moreDescription,SIGNAL(linkActivated(QString)),this,SLOT(showMoreBio()));

    ui->widget_artist->setLayout(new QVBoxLayout);
    ui->widget_tags->setLayout(new QVBoxLayout);
    ui->widget_tracks->setLayout(new QVBoxLayout);
    ui->widget_comments->setLayout(new QVBoxLayout);
    ui->widget_comments->layout()->setSpacing(0);

    connect(ui->label_moreTracks,SIGNAL(linkActivated(QString)),this,SLOT(loadTracks()));

    connect(ui->label_moreTags,SIGNAL(linkActivated(QString)),this,SLOT(loadTags()));

//    connect(ui->label_moreArtists,SIGNAL(linkActivated(QString)),this,SLOT(loadSimilar()));

    connect(ui->label_moreShoutbox,SIGNAL(linkActivated(QString)),this,SLOT(loadShouts()));
    connect(ui->textEdit_shout,SIGNAL(textChanged()),this,SLOT(evalShout()));
    connect(ui->pushButton_post,SIGNAL(clicked()),this,SLOT(sendShout()));

    ui->label_albumPic->adjustSize();
    ui->frame_art->adjustSize();
    ui->frame_header->adjustSize();

    connect(ui->button_play,SIGNAL(clicked()),this,SLOT(play()));
    s_albumLoadCount=0;
    s_trackLoadCount=0;
    s_tagLoadCount=0;

    loadArtist();
    loadTracks();
    loadTags();

    readjustPriorities();
}

HAlbumContext::~HAlbumContext()
{
    delete ui;
}

void HAlbumContext::showEvent(QShowEvent * e)
{
    s_priority[0].push_back(s_rep.sendPic(HAlbum::Large,this,"setPic"));
    s_priority[3].push_back(HUser::get(lastfm::ws::Username).sendPic(HUser::Medium,this,"setMePic"));
    readjustPriorities();
    loadShouts();
    QWidget::showEvent(e);
}

void HAlbumContext::hideEvent(QHideEvent *e) {
    readjustPriorities();
    QWidget::hideEvent(e);
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
    s_priority[0].push_back(s_rep.sendContent(this,"setSummary"));
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
    s_priority[0].push_back(s_rep.sendTracks(this,"loadTracks_2"));
}

void HAlbumContext::loadTracks_2(HTrack* track)
{
    if(track) {
        HTrackBox* ab=HTrackBox::getBox(*track);
        if(s_showTime.msecsTo(QTime::currentTime())>110) {
            QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
            pa->setStartValue(0);
            pa->setEndValue(32);
            pa->setDuration(500);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }
        ui->widget_tracks->layout()->addWidget(ab);
    } else {
        ui->label_moreTracks->hide();
    }
    s_trackLoadCount++;
}

void HAlbumContext::loadTags()
{
    ui->label_moreTags->setText("<p align=\"right\"><i>Loading...</i></p>");
    if (s_tagLoadCount) s_priority[1].push_back(s_rep.sendMoreTags(this,"addTags"));
    else s_priority[1].push_back(s_rep.sendTags(this, "addTags"));
}

void HAlbumContext::loadShouts(int s)
{
    ui->label_moreShoutbox->setText("<p align=\"right\"><i>Loading...</i></p>");
    if(s==-1) s_priority[1].push_back(s_rep.sendShouts(this,"setShouts",s_shoutsToLoad));
    else {
        s_shoutLoadCount=0;
        s_shoutsToLoad=5;
        s_priority[3].push_back(s_rep.sendShouts(this,"setShouts",s));
    }
    readjustPriorities();
}

void HAlbumContext::play() {
    KFadeWidgetEffect* fwe=new KFadeWidgetEffect(this);
    setGraphicsEffect(s_ge=new QGraphicsBlurEffect(this));
    setEnabled(0);
    if(!s_pw) {
        s_pw = new HPlayWidget(s_rep,this->parentWidget());
    } else s_pw->reset();
    s_pw->setGeometry(parentWidget()->width()/2-s_pw->geometry().width()/2,50,s_pw->geometry().width(),s_pw->geometry().height());
    s_pw->adjustSize();
    s_pw->show();
    connect(s_pw,SIGNAL(closed()),this,SLOT(hidePlay()));
    fwe->start();
}

void HAlbumContext::hidePlay() {
    KFadeWidgetEffect* fwe=new KFadeWidgetEffect(this);
    delete s_ge;
    s_ge=0;
    s_pw->hide();
    setEnabled(1);
    fwe->start();
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

void HAlbumContext::setPic(QImage& p) {
    if(!isVisible()) return;
    ui->label_albumPic->setPixmap(p);
    ui->label_albumPic->setMinimumSize(p.size());
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

void HAlbumContext::setMePic(QImage& pic) {
    if(pic.width()!=70) pic=pic.scaledToWidth(70,Qt::SmoothTransformation);
    ui->label_you->setPixmap(pic);
}

void HAlbumContext::setShouts(HShout* shouts) {
    if(!shouts) {
        ui->label_moreShoutbox->setText(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
        return;
    }
    if(s_loadedShouts.contains(shouts)) return;
    s_loadedShouts.push_back(shouts);
    {
        HShoutBox* ab=new HShoutBox(*shouts,this);
        if(s_showTime.msecsTo(QTime::currentTime())>110) {
            ab->setFixedHeight(0);
            ab->adjustSize();
            QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
            pa->setStartValue(0);
            pa->setEndValue(ab->sizeHint().height());
            pa->setDuration(500);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }
        ab->adjustSize();
        ui->widget_comments->layout()->addWidget(ab);
    }
//    if(i-s_shoutCount!=toLoad) {
//        ui->label_moreArtists->hide();
    /*} else*/ {
    }
    s_shoutLoadCount++;
    s_shoutsToLoad+=2;
}

void HAlbumContext::addTags(QList<HTag*> tags) {
    int i;
    int toLoad=s_tagLoadCount?s_tagLoadCount*2:4;
    for(i=s_tagLoadCount;i<tags.size()&&i-s_tagLoadCount<toLoad;i++) {
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

void HAlbumContext::evalShout() {
    ui->pushButton_post->setEnabled(ui->textEdit_shout->toPlainText().size()&&ui->textEdit_shout->toPlainText().size()<1000);
    ui->label_characterUse->setText(QString::number(ui->textEdit_shout->toPlainText().size())+"/1000 characters used");
}

void HAlbumContext::sendShout() {
    if(HLfmWebManager::singleton()) HLfmWebManager::singleton()->shout("http://www.last.fm/music/"+s_rep.getArtistName()+"/"+s_rep.getAlbumName(),ui->textEdit_shout->toPlainText());
    ui->textEdit_shout->setText("");
    ui->label_characterUse->setText("Sent!");
}
