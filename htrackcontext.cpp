#include "htrackcontext.h"
#include "ui_htrackcontext.h"
#include "htrackbox.h"
#include "htrackbox.h"
#include "htagbox.h"
#include "hshoutbox.h"
#include "hartistbox.h"
#include "halbumbox.h"
#include <QRect>
#include <QScrollBar>
#include <QMenu>

#include "hrdiointerface.h"

HTrackContext::HTrackContext(HTrack& rep, QWidget *parent) :
    QWidget(parent),
    s_rep(rep),
    s_albumLoadCount(0),
    s_artistLoadCount(0),
    s_trackLoadCount(0),
    s_tagLoadCount(0),
    s_similarLoadCount(0),
    s_shoutLoadCount(0),
    ui(new Ui::HTrackContext)
{
    ui->setupUi(this);
    ui->label_description->setText(s_rep.getSummary().size()?s_rep.getSummary():"No article is available for this track.");
    ui->label_playcount->setText("<B>"+QString::number(s_rep.getPlayCount())+"</B> plays by <B>"+QString::number(s_rep.getListenerCount())+"</B> users");
    ui->label_userplaycount->setText("<B>"+QString::number(s_rep.getUserPlayCount())+"</B> plays in your library");
    ui->label_track->setText(s_rep.getTrackName());

    connect(ui->label_moreDescription,SIGNAL(linkActivated(QString)),this,SLOT(showMoreBio()));

    connect(ui->button_play,SIGNAL(clicked()),this,SLOT(playTrack()));
    connect(ui->button_more,SIGNAL(clicked()),this,SLOT(playSimilar()));

    ui->widget_artist->setLayout(new QVBoxLayout);
    ui->widget_albums->setLayout(new QVBoxLayout);
    ui->widget_tags->setLayout(new QVBoxLayout);
    ui->widget_similar->setLayout(new QVBoxLayout);
    ui->widget_comments->setLayout(new QVBoxLayout);

    QTimer::singleShot(300,this,SLOT(loadArtist()));
    connect(ui->label_moreTracks,SIGNAL(linkActivated(QString)),this,SLOT(loadSimilar()));

    connect(ui->label_moreTags,SIGNAL(linkActivated(QString)),this,SLOT(loadTags()));

    connect(ui->label_moreShoutbox,SIGNAL(linkActivated(QString)),this,SLOT(loadShouts()));

    ui->label_you->setPixmap(HUser::get(lastfm::ws::Username).getPic(HUser::Medium));

    ui->frame_header->adjustSize();

    // menus
    QMenu* playMenu=new QMenu;
    playMenu->addAction("Queue",this,SLOT(playTrack()));
    playMenu->addAction("Replace",this,SLOT(playReplacing()));
    ui->button_play->setMenu(playMenu);

    QMenu* moreMenu=new QMenu;
    moreMenu->addAction("Queue five similar tracks",this,SLOT(playSimilar()));
    moreMenu->addAction("Queue ten similar tracks",this,SLOT(playMoreSimilar()));
    moreMenu->addAction("Replace queue with ten songs",this,SLOT(playMoreSimilarReplacing()));
    ui->button_more->setMenu(moreMenu);
}

HTrackContext::~HTrackContext()
{
    delete ui;
}

void HTrackContext::showMoreBio()
{
    ui->label_moreDescription->setText("Loading...");
    ui->frame_header->adjustSize();
    ui->label_description->adjustSize();
    ui->label_description->setFixedHeight(ui->label_description->height());
    ui->label_description->setText(s_rep.getContent());
    ui->label_description->adjustSize();

    ui->label_moreDescription->hide();
}

void HTrackContext::loadArtist()
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
        QTimer::singleShot(0,this,SLOT(loadAlbum()));
    }
    s_artistLoadCount=1;
    ui->label_moreArtists->hide();
}

void HTrackContext::loadAlbum()
{
    ui->label_moreAlbums->setText("<p align=\"right\"><i>Loading...</i></p>");
    if(s_rep.getAlbums().size())
    {
        s_rep.getAlbums()[0]->getPic(HAlbum::Large);   //CACHE
        HAlbumBox* ab=new HAlbumBox(*s_rep.getAlbums()[0]);
        QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
        pa->setStartValue(0);
        pa->setEndValue(ab->sizeHint().height());
        pa->setDuration(500);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
        ui->widget_albums->layout()->addWidget(ab);
    }
    if(!s_albumLoadCount) {
        QTimer::singleShot(0,this,SLOT(loadSimilar()));
    }
    s_albumLoadCount=1;
    ui->label_moreArtists->hide();
}

void HTrackContext::loadTags()
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

void HTrackContext::loadShouts()
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

void HTrackContext::loadSimilar()
{
    ui->label_moreTracks->setText("<p align=\"right\"><i>Loading...</i></p>");
    QList<HTrack*> tracks=s_rep.getSimilar();
    int i;
    int toLoad=s_similarLoadCount?s_similarLoadCount*2:4;
    for(i=s_similarLoadCount;i<tracks.size()&&i-s_similarLoadCount<toLoad;i++) {
        tracks[i]->getContent();    //CACHE
        HTrackBox* ab=new HTrackBox(*tracks[i]);
        QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
        pa->setStartValue(0);
        pa->setEndValue(40);
        pa->setDuration(500);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
        ui->widget_similar->layout()->addWidget(ab);
    }
    if(i-s_similarLoadCount!=toLoad) {
        ui->label_moreTracks->hide();
    } else {
        ui->label_moreTracks->setText(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
    }

    if(!s_similarLoadCount) {
        QTimer::singleShot(0,this,SLOT(loadTags()));
    }
    s_similarLoadCount=i;
}

void HTrackContext::playTrack() {
    HRdioInterface::singleton()->queue(s_rep);
}

void HTrackContext::playReplacing() {
    HRdioInterface::singleton()->play(s_rep);

}

void HTrackContext::playSimilar() {
    HRdioInterface::singleton()->queue(s_rep);
    for(int i=0;i<5&&i<s_rep.getSimilar().size();i++) {
        HRdioInterface::singleton()->queue(*s_rep.getSimilar()[i]);
    }
}
void HTrackContext::playMoreSimilar() {
    playSimilar();

    for(int i=5;i<10&&i<s_rep.getSimilar().size();i++) {
        HRdioInterface::singleton()->queue(*s_rep.getSimilar()[i]);
    }
}
void HTrackContext::playMoreSimilarReplacing() {
    playReplacing();

    for(int i=0;i<10&&i<s_rep.getSimilar().size();i++) {
        HRdioInterface::singleton()->queue(*s_rep.getSimilar()[i]);
    }
}
