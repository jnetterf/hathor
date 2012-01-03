#include "halbumcontext.h"
#include "ui_halbumcontext.h"
#include "halbumbox.h"
#include "htrackbox.h"
#include "htagbox.h"
#include "halbumbox.h"
#include "hshoutbox.h"
#include "hartistbox.h"
#include "hrdiointerface.h"
#include <QRect>
#include <QScrollBar>
#include <QMenu>

HAlbumContext::HAlbumContext(HAlbum& rep, QWidget *parent) :
    QWidget(parent),
    s_rep(rep),
    s_albumLoadCount(0),
    s_trackLoadCount(0),
    s_tagLoadCount(0),
    s_artistLoadCount(0),
    s_shoutLoadCount(0),
    ui(new Ui::HAlbumContext)
{
    ui->setupUi(this);
    ui->label_album->setText(s_rep.getAlbumName());
    ui->label_artist->setText("by "+s_rep.getArtistName());
    ui->label_albumPic->setPixmap(s_rep.getPic(HAlbum::Large));
    ui->label_description->setText(s_rep.getSummary().size()?s_rep.getSummary():"Sorry, no article is available for this album. :(");
    ui->label_playcount->setText("<B>"+QString::number(s_rep.getPlayCount())+"</B> plays by <B>"+QString::number(s_rep.getListenerCount())+"</B> users");
    ui->label_userplaycount->setText("<B>"+QString::number(s_rep.getUserPlayCount())+"</B> plays in your library");

    connect(ui->label_moreDescription,SIGNAL(linkActivated(QString)),this,SLOT(showMoreBio()));

    ui->widget_artist->setLayout(new QVBoxLayout);
    ui->widget_tags->setLayout(new QVBoxLayout);
    ui->widget_tracks->setLayout(new QVBoxLayout);
    ui->widget_comments->setLayout(new QVBoxLayout);

    QTimer::singleShot(300,this,SLOT(loadArtist()));
    connect(ui->label_moreTracks,SIGNAL(linkActivated(QString)),this,SLOT(loadTracks()));

    connect(ui->label_moreTags,SIGNAL(linkActivated(QString)),this,SLOT(loadTags()));

//    connect(ui->label_moreArtists,SIGNAL(linkActivated(QString)),this,SLOT(loadSimilar()));

    connect(ui->label_moreShoutbox,SIGNAL(linkActivated(QString)),this,SLOT(loadShouts()));

    ui->label_you->setPixmap(HUser::get(lastfm::ws::Username).getPic(HUser::Medium));

    ui->label_albumPic->adjustSize();
    ui->frame_art->adjustSize();
    ui->frame_header->adjustSize();

    QMenu* playMenu=new QMenu();
    playMenu->addAction("Queue album",this,SLOT(play()));
    playMenu->addAction("Replace queue with album",this,SLOT(playReplacing()));
    ui->button_play->setMenu(playMenu);
    connect(ui->button_play,SIGNAL(clicked()),this,SLOT(play()));
}

HAlbumContext::~HAlbumContext()
{
    delete ui;
}

void HAlbumContext::showMoreBio()
{
    ui->label_moreDescription->setText("Loading...");
    int deltaWidth=-ui->label_albumPic->pixmap()->width();
    int deltaHeight=-ui->label_albumPic->pixmap()->height();
//    ui->label_albumPic->setPixmap(s_rep.getPic(HAlbum::Mega));
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
    ui->label_description->setFixedHeight(ui->label_description->height());
    ui->label_description->setText(s_rep.getContent());
    ui->label_description->adjustSize();

    {
        QPropertyAnimation* pa=new QPropertyAnimation(ui->label_description,"maximumHeight");
        pa->setStartValue(ui->label_description->height());
        pa->setEndValue(ui->label_description->heightForWidth(ui->label_description->width()-deltaWidth));
        pa->setDuration((ui->label_description->heightForWidth(ui->label_description->width()-deltaWidth)-ui->label_description->height())*2);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
    }
    ui->label_moreDescription->hide();
}

void HAlbumContext::loadArtist()
{
    ui->label_moreArtists->setText("<p align=\"right\"><i>Loading...</i></p>");
    {
        s_rep.getArtist().getPic(HArtist::Large);   //CACHE
        HArtistBox* ab=new HArtistBox(s_rep.getArtist());
        QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
        pa->setStartValue(0);
        pa->setEndValue(ab->sizeHint().height());
        pa->setDuration(500);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
        ui->widget_artist->layout()->addWidget(ab);
    }
    if(!s_artistLoadCount) {
        QTimer::singleShot(0,this,SLOT(loadTracks()));
    }
    s_artistLoadCount=1;
    ui->label_moreArtists->hide();
}

void HAlbumContext::loadTracks()
{
    ui->label_moreTracks->setText("<p align=\"right\"><i>Loading...</i></p>");
    QList<HTrack*> tracks=s_rep.getTracks();
    int i;
    int toLoad=s_trackLoadCount?s_trackLoadCount*2:10;
    for(i=s_trackLoadCount;i<tracks.size()&&i-s_trackLoadCount<toLoad;i++) {
        tracks[i]->getContent();    //CACHE
        HTrackBox* ab=new HTrackBox(*tracks[i]);
        QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
        pa->setStartValue(0);
        pa->setEndValue(32);
        pa->setDuration(500);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
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
    if(!s_trackLoadCount) {
        QTimer::singleShot(0,this,SLOT(loadTags()));
    }
    s_trackLoadCount=i;
}

void HAlbumContext::loadTags()
{
    ui->label_moreTags->setText("<p align=\"right\"><i>Loading...</i></p>");
    QList<HTag*> tags=s_tagLoadCount?s_rep.getMoreTags():s_rep.getTags();
    int i;
    int toLoad=s_tagLoadCount?s_tagLoadCount*2:4;
    for(i=s_tagLoadCount;i<tags.size()&&i-s_tagLoadCount<toLoad;i++) {
        tags[i]->getContent();    //CACHE
        HTagBox* ab=new HTagBox(*tags[i]);
        QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
        pa->setStartValue(0);
        pa->setEndValue(40);
        pa->setDuration(500);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
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
    if(!s_tagLoadCount) {
        QTimer::singleShot(0,this,SLOT(loadShouts()));
    }
    s_tagLoadCount=i;
}

void HAlbumContext::loadShouts()
{
    ui->label_moreShoutbox->setText("<p align=\"right\"><i>Loading...</i></p>");
    QList<HShout*> shouts=s_rep.getShouts();
    int i;
    int toLoad=s_shoutLoadCount?s_shoutLoadCount*2:10;
    for(i=s_shoutLoadCount;i<shouts.size()&&i-s_shoutLoadCount<toLoad;i++) {
        shouts[i]->getShouter().getPic(HUser::Medium);    //CACHE
        HShoutBox* ab=new HShoutBox(*shouts[i]);
        {
            QPropertyAnimation* pa=new QPropertyAnimation(ab,"minimumHeight");
            pa->setStartValue(0);
            pa->setEndValue(ab->heightForWidth(ui->widget_comments->width()-20));
            pa->setDuration(500);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }
        {
            QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
            pa->setStartValue(0);
            pa->setEndValue(ab->heightForWidth(ui->widget_comments->width()-20));
            pa->setDuration(500);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }
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

void HAlbumContext::play() {
    HRdioInterface::singleton()->queue(s_rep);
}

void HAlbumContext::playReplacing() {
    HRdioInterface::singleton()->play(s_rep);
}
