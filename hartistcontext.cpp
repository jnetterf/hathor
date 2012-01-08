#include "hartistcontext.h"
#include "ui_artistcontext.h"
#include "halbumbox.h"
#include "hrdiointerface.h"
#include "htrackbox.h"
#include "htagbox.h"
#include "hartistbox.h"
#include "hshoutbox.h"
#include "kfadewidgeteffect.h"
#include <QRect>
#include <QScrollBar>
#include <QMenu>

HArtistContext::HArtistContext(HArtist& rep, QWidget *parent) :
    QWidget(parent),
    s_rep(rep),
    s_albumLoadCount(0),
    s_trackLoadCount(0),
    s_tagLoadCount(0),
    s_similarLoadCount(0),
    s_shoutLoadCount(0),
    ui(new Ui::HArtistContext)
{
    ui->setupUi(this);
    ui->label_artist->setText(s_rep.getName());
    ui->label_artistPic->setPixmap(s_rep.getPic(HArtist::Large));
    ui->label_description->setText(s_rep.getBioShort());
    ui->label_playcount->setText("<B>"+QString::number(s_rep.getPlayCount())+"</B> plays by <B>"+QString::number(s_rep.getListenerCount())+"</B> users");
    ui->label_userplaycount->setText("<B>"+QString::number(s_rep.getUserPlayCount())+"</B> plays in your library");

    connect(ui->label_moreDescription,SIGNAL(linkActivated(QString)),this,SLOT(showMoreBio()));

    QVBoxLayout* vb;
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_albums->setLayout(vb);
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_tracks->setLayout(vb);
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_tags->setLayout(vb);
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_similarArtists->setLayout(vb);
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_comments->setLayout(vb);

    QTimer::singleShot(300,this,SLOT(loadAlbums()));
    connect(ui->label_moreAlbums,SIGNAL(linkActivated(QString)),this,SLOT(loadAlbums()));

    connect(ui->label_moreTracks,SIGNAL(linkActivated(QString)),this,SLOT(loadTracks()));

    connect(ui->label_moreTags,SIGNAL(linkActivated(QString)),this,SLOT(loadTags()));

    connect(ui->label_moreArtists,SIGNAL(linkActivated(QString)),this,SLOT(loadSimilar()));

    connect(ui->label_moreShoutbox,SIGNAL(linkActivated(QString)),this,SLOT(loadShouts()));

    ui->label_you->setPixmap(HUser::get(lastfm::ws::Username).getPic(HUser::Medium).scaledToWidth(70,Qt::SmoothTransformation));

    connect(ui->textEdit,SIGNAL(textChanged()),this,SLOT(evalShout()));
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(sendShout()));

    ui->frame_header->adjustSize();
    ui->label_artistPic->adjustSize();
    ui->frame_art->adjustSize();

    //MENUS
    QMenu* playMenu=new QMenu();
    playMenu->addAction("Queue top 5 tracks",this,SLOT(play()));
    playMenu->addAction("Queue top 10 tracks",this,SLOT(playMore()));
    playMenu->addAction("Replace queue with top 10 tracks",this,SLOT(playMoreReplacing()));
    ui->button_play->setMenu(playMenu);
    connect(ui->button_play,SIGNAL(clicked()),this,SLOT(play()));

    QMenu* addMenu=new QMenu();
    addMenu->addAction("Queue top track of 5 similar artists",this,SLOT(add()));
    addMenu->addAction("Queue 2 top tracks from 10 similar artists",this,SLOT(addMore()));
    addMenu->addAction("Queue 5 top tracks from 10 similar artists",this,SLOT(addEvenMore()));
    addMenu->addAction("Queue 10 top tracks from 10 similar artists",this,SLOT(addMost()));
    addMenu->addAction("Replace queue with 10 top tracks from 10 similar artists",this,SLOT(addMostReplacing()));
    ui->button_more->setMenu(addMenu);
    connect(ui->button_more,SIGNAL(clicked()),this,SLOT(add()));
}

HArtistContext::~HArtistContext()
{
    delete ui;
}

void HArtistContext::showMoreBio()
{
    ui->label_moreDescription->setText("Loading...");
    int deltaWidth=-ui->label_artistPic->pixmap()->width();
    int deltaHeight=-ui->label_artistPic->pixmap()->height();
    ui->label_artistPic->setPixmap(s_rep.getPic(HArtist::Mega));
    deltaWidth+=ui->label_artistPic->pixmap()->width();
    deltaHeight+=ui->label_artistPic->pixmap()->height();
    QPropertyAnimation* pa1=new QPropertyAnimation(ui->label_artistPic,"maximumSize");
    pa1->setStartValue(QSize(ui->label_artistPic->pixmap()->width()-deltaWidth,ui->label_artistPic->pixmap()->height()-deltaHeight));
    pa1->setEndValue(QSize(ui->label_artistPic->pixmap()->width(),ui->label_artistPic->pixmap()->height()));
    pa1->setDuration(300);
    pa1->start(QPropertyAnimation::DeleteWhenStopped);
    ui->label_artistPic->adjustSize();
    ui->frame_art->adjustSize();
    ui->label_description->adjustSize();
    ui->label_description->setFixedHeight(ui->label_description->height());
    ui->label_description->setText(s_rep.getBio());
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

void HArtistContext::loadAlbums()
{
    ui->label_moreAlbums->setText("<p align=\"right\"><i>Loading...</i></p>");
    QList<HAlbum*> albums=s_rep.getAlbums();
    int i;
    int toLoad=s_albumLoadCount?s_albumLoadCount*2:3;
    for(i=s_albumLoadCount;i<albums.size()&&i-s_albumLoadCount<toLoad;i++) {
        albums[i]->getPic(HAlbum::Large);   //CACHE
        HAlbumBox* ab=new HAlbumBox(*albums[i]);
        QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
        pa->setStartValue(0);
        pa->setEndValue(ab->heightForWidth(300));
        pa->setDuration(500);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
        ui->widget_albums->layout()->addWidget(ab);
        ui->widget_albums->layout()->setAlignment(ab,Qt::AlignTop);
    }
    if(i-s_albumLoadCount!=toLoad) {
        ui->label_moreAlbums->hide();
    } else {
        ui->label_moreAlbums->setText(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
    }
    if(!s_albumLoadCount) {
        QTimer::singleShot(0,this,SLOT(loadSimilar()));
    }
    s_albumLoadCount=i;
}

void HArtistContext::loadTracks()
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


void HArtistContext::loadTags()
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


void HArtistContext::loadSimilar()
{
    ui->label_moreArtists->setText("<p align=\"right\"><i>Loading...</i></p>");
    QList<HArtist*> similar=s_rep.getSimilar();
    int i;
    int toLoad=s_similarLoadCount?s_similarLoadCount*2:4;
    for(i=s_similarLoadCount;i<similar.size()&&i-s_similarLoadCount<toLoad;i++) {
        similar[i]->getBioShort();    //CACHE
        HArtistBox* ab=new HArtistBox(*similar[i]);
        QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
        pa->setStartValue(0);
        pa->setEndValue(ab->sizeHint().height());
        pa->setDuration(500);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
        ui->widget_similarArtists->layout()->addWidget(ab);
    }
    if(i-s_similarLoadCount!=toLoad) {
        ui->label_moreArtists->hide();
    } else {
        ui->label_moreArtists->setText(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
    }
    if(!s_similarLoadCount) {
        QTimer::singleShot(0,this,SLOT(loadTracks()));
    }
    s_similarLoadCount=i;
}

void HArtistContext::loadShouts()
{
    ui->label_moreShoutbox->setText("<p align=\"right\"><i>Loading...</i></p>");
    QList<HShout*> shouts=s_rep.getShouts();
    int i;
    int toLoad=s_shoutLoadCount?s_shoutLoadCount*2:10;
    for(i=s_shoutLoadCount;i<shouts.size()&&i-s_shoutLoadCount<toLoad;i++) {
        shouts[i]->getShouter().getPic(HUser::Medium);    //CACHE
        HShoutBox* ab=new HShoutBox(*shouts[i]);
        {
            QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
            pa->setStartValue(0);
            pa->setEndValue(ab->heightForWidth(ui->widget_comments->width()-20));
            pa->setDuration(500);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }
        ui->widget_comments->layout()->addWidget(ab);
        ui->widget_albums->layout()->setAlignment(ab,Qt::AlignTop);

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

void HArtistContext::play() {
    for(int i=0;i<s_rep.getTracks().size()&&i<5;i++) {
        HRdioInterface::singleton()->queue(*s_rep.getTracks()[i]);
    }
}

void HArtistContext::playMore() {
    for(int i=0;i<s_rep.getTracks().size()&&i<10;i++) {
        HRdioInterface::singleton()->queue(*s_rep.getTracks()[i]);
    }
}

void HArtistContext::playMoreReplacing() {
    for(int i=0;i<s_rep.getTracks().size()&&i<10;i++) {
        if(!i) HRdioInterface::singleton()->play(*s_rep.getTracks()[i]);
        else HRdioInterface::singleton()->queue(*s_rep.getTracks()[i]);
    }
}

//1-5,2-10,5-10,10-10
void HArtistContext::add(int a,int b) {
    for(int i=0;i<s_rep.getSimilar().size()&&i<b;i++) {
        for(int j=0;j<a&&j<s_rep.getSimilar()[i]->getTracks().size();j++) {
            HRdioInterface::singleton()->queue(*s_rep.getSimilar()[i]->getTracks()[j]);
        }
    }
}

void HArtistContext::addMore() {
    add(2,10);
}

void HArtistContext::addEvenMore() {
    add(5,10);
}

void HArtistContext::addMost() {
    add(10,10);
}

void HArtistContext::addMostReplacing() {
    int a=10, b=10;
    for(int i=0;i<s_rep.getSimilar().size()&&i<b;i++) {
        for(int j=0;j<a&&j<s_rep.getSimilar()[i]->getTracks().size();j++) {
            if(!i&&!j) HRdioInterface::singleton()->play(*s_rep.getSimilar()[i]->getTracks()[j]);
            else HRdioInterface::singleton()->queue(*s_rep.getSimilar()[i]->getTracks()[j]);
        }
    }
}

void HArtistContext::evalShout() {
    ui->pushButton->setEnabled(ui->textEdit->toPlainText().size()&&ui->textEdit->toPlainText().size()<1000);
    ui->label_wordCount->setText(QString::number(ui->textEdit->toPlainText().size())+"/1000 characters used");
}

void HArtistContext::sendShout() {
    QMap<QString, QString> params;
    params["method"] = "artist.shout";
    params["artist"] = s_rep.getName();
    params["message"] = ui->textEdit->toPlainText();

    QNetworkReply* reply = lastfmext_post( params );
    QEventLoop loop;
    QTimer::singleShot(2850,&loop,SLOT(quit()));
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    ui->textEdit->setText("");
    ui->label_wordCount->setText("Sent!");
}
