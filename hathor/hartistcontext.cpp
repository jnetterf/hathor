#include "hartistcontext.h"
#include "ui_artistcontext.h"
#include "halbumbox.h"
#include "htrackbox.h"
#include "htagbox.h"
#include "hartistbox.h"
#include "hshoutbox.h"
#include "kfadewidgeteffect.h"
#include "hplaywidget.h"
#include "hnettloger.h"
#include <QRect>
#include <QScrollBar>
#include <QGraphicsBlurEffect>
#include <QMenu>

HArtistContext* HArtistContext::getContext(HArtist &rep) {
    return new HArtistContext(rep);
}

HArtistContext::HArtistContext(HArtist& rep, QWidget *parent) :
    QWidget(parent),
    s_rep(rep),
    s_albumLoadCount(0),
    s_trackLoadCount(0),
    s_tagLoadCount(0),
    s_similarLoadCount(0),
    s_shoutLoadCount(0),
    s_playCountCache(0),
    s_listenerCountCache(0),
    s_userPlayCountCache(0),
    deltaWidth(0),
    s_tagsToLoad(0),
    s_ge(0),
    s_pw(0),
    s_albumsToLoad(3),
    s_tracksToLoad(10),
    s_similarToLoad(4),
    s_shoutsToLoad(5),
    s_bigPic(0),
    ui(new Ui::HArtistContext)
{
    s_showTime=QTime::currentTime();

    ui->setupUi(this);
    ui->label_artist->setText(s_rep.getName());

    connect(ui->label_moreDescription,SIGNAL(linkActivated(QString)),this,SLOT(showMoreBio()));

    QVBoxLayout* vb;
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_albums->setLayout(vb);
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_tracks->setLayout(vb);
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_tags->setLayout(vb);
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_similarArtists->setLayout(vb);
    vb=new QVBoxLayout; vb->setSpacing(0); ui->widget_comments->setLayout(vb);

    QTimer::singleShot(00,this,SLOT(continueLoading()));
//    QTimer::singleShot(01,this,SLOT(loadAlbums()));
//    QTimer::singleShot(02,this,SLOT(loadShouts()));
//    QTimer::singleShot(03,this,SLOT(loadSimilar()));
//    QTimer::singleShot(04,this,SLOT(loadTags()));
//    QTimer::singleShot(05,this,SLOT(loadTracks()));

    connect(ui->label_moreAlbums,SIGNAL(linkActivated(QString)),this,SLOT(loadAlbums()));

    connect(ui->label_moreTracks,SIGNAL(linkActivated(QString)),this,SLOT(loadTracks()));

    connect(ui->label_moreTags,SIGNAL(linkActivated(QString)),this,SLOT(loadTags()));

    connect(ui->label_moreArtists,SIGNAL(linkActivated(QString)),this,SLOT(loadSimilar()));

    connect(ui->label_moreShoutbox,SIGNAL(linkActivated(QString)),this,SLOT(loadShouts()));

    connect(ui->textEdit,SIGNAL(textChanged()),this,SLOT(evalShout()));
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(sendShout()));

    ui->frame_header->adjustSize();
    ui->label_artistPic->adjustSize();
    ui->frame_art->adjustSize();

    connect(ui->button_play,SIGNAL(clicked()),this,SLOT(play()));
    adjustSize();
    readjustPriorities();
}

void HArtistContext::continueLoading() {
    s_priority[0].push_back(s_rep.sendBioShort(this,"setBio"));
    s_priority[0].push_back(s_rep.sendPlayCount(this,"setPlayCount"));
    s_priority[0].push_back(s_rep.sendListenerCount(this,"setListenerCount"));
    s_priority[0].push_back(s_rep.sendUserPlayCount(this,"setUserPlayCount"));

    s_showTime=QTime::currentTime();
    loadAlbums(s_albumLoadCount?s_albumLoadCount:s_albumsToLoad);
    loadTracks(s_trackLoadCount?s_trackLoadCount:s_tracksToLoad);
    loadSimilar(s_similarLoadCount?s_similarLoadCount:s_similarToLoad);
    loadShouts(s_similarLoadCount?s_similarLoadCount:s_similarToLoad);
    loadTags(s_tagLoadCount);

    resizeEvent(0);
    readjustPriorities();


    readjustPriorities();
}


void HArtistContext::showEvent(QShowEvent * e) {
    ui->label_artistPic->setPixmap(0);
    s_priority[3].push_back(HUser::get(lastfm::ws::Username).sendPic(HUser::Medium,this,"setMePic"));
    s_priority[1].push_back(s_rep.sendPic(s_bigPic?HArtist::Mega:HArtist::Large,this,"setPic"));
    readjustPriorities();
    QWidget::showEvent(e);
}

void HArtistContext::hideEvent(QHideEvent *e) {
    QWidget::hideEvent(e);
    readjustPriorities();
}

void HArtistContext::resizeEvent(QResizeEvent *e) {
    if(s_pw) {
        s_pw->setGeometry(parentWidget()->width()/2-s_pw->geometry().width()/2,50,s_pw->geometry().width(),s_pw->geometry().height());
        s_pw->adjustSize();
    }
    ui->scrollAreaWidgetContents->adjustSize();
    ui->scrollAreaWidgetContents_2->adjustSize();
    if(e) QWidget::resizeEvent(e);
}

HArtistContext::~HArtistContext()
{
    delete ui;
}

void HArtistContext::showMoreBio()
{
    ui->label_moreDescription->setText("Loading...");
    s_bigPic=1;
    s_priority[0].push_back(s_rep.sendPic(HArtist::Mega,this,"setPic"));
    s_priority[0].push_back(s_rep.sendBio(this,"setBio"));
    ui->label_moreDescription->hide();
    readjustPriorities();
}

void HArtistContext::loadAlbums(int s)
{
    ui->label_moreAlbums->setText("<p align=\"right\"><i>Loading...</i></p>");
    if(s==-1) s_priority[1].push_back(s_rep.sendAlbums(this,"setAlbums",s_albumsToLoad));
    else {
        s_albumLoadCount=0;
        s_albumsToLoad=3;
        s_priority[1].push_back(s_rep.sendAlbums(this,"setAlbums",s));
    }
    readjustPriorities();
}

void HArtistContext::loadTracks(int s)
{
    ui->label_moreTracks->setText("<p align=\"right\"><i>Loading...</i></p>");
    if(s==-1) s_priority[2].push_back(s_rep.sendTracks(this,"setTracks",s_tracksToLoad));
    else {
        s_trackLoadCount=0;
        s_tracksToLoad=10;
        s_priority[2].push_back(s_rep.sendTracks(this,"setTracks",s));
    }
    readjustPriorities();
}


void HArtistContext::loadTags(int s)
{
    if(s!=-1) {
        s_tagLoadCount=0;
    }
    ui->label_moreTags->setText("<p align=\"right\"><i>Loading...</i></p>");
    if(s_tagLoadCount) s_priority[2].push_back(s_rep.sendMoreTags(this,"setTags"));
    else s_priority[2].push_back(s_rep.sendTags(this,"setTags"));
    readjustPriorities();
}


void HArtistContext::loadSimilar(int s)
{
    ui->label_moreArtists->setText("<p align=\"right\"><i>Loading...</i></p>");
    if(s==-1) s_priority[1].push_back(s_rep.sendSimilar(this,"setSimilar",s_similarToLoad));
    else {
        s_similarLoadCount=0;
        s_similarToLoad=4;
        s_priority[1].push_back(s_rep.sendSimilar(this,"setSimilar",s));
    }
    readjustPriorities();
}

void HArtistContext::loadShouts(int s)
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

void HArtistContext::play() {
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

void HArtistContext::hidePlay() {
    KFadeWidgetEffect* fwe=new KFadeWidgetEffect(this);
    delete s_ge;
    s_ge=0;
    s_pw->hide();
    setEnabled(1);
    fwe->start();

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
    QTimer::singleShot(5000,reply,SLOT(deleteLater()));
    connect(reply,SIGNAL(finished()),this,SLOT(onShoutSent()));
}

void HArtistContext::onShoutSent() {
    Q_ASSERT(sender());
    if(sender()) sender()->deleteLater();
    ui->textEdit->setText("");
    ui->label_wordCount->setText("Sent!");
}

void HArtistContext::setPic(QImage& p) {
    if(!isVisible()) return;
    if(ui->label_artistPic->pixmap()) deltaWidth=-ui->label_artistPic->pixmap()->width();
    else deltaWidth=0;
    int deltaHeight;
    if(ui->label_artistPic->pixmap()) deltaHeight=-ui->label_artistPic->pixmap()->height();
    else deltaHeight=0;
    bool a=ui->label_artistPic->pixmap();
    ui->label_artistPic->setPixmap(p);
    deltaWidth+=p.width();
    deltaHeight+=p.height();

    if(a) {
        {
            QPropertyAnimation* pa1=new QPropertyAnimation(ui->label_artistPic,"maximumSize");
            pa1->setStartValue(QSize(qMax(0,ui->label_artistPic->width()-deltaWidth),
                                     qMax(0,ui->label_artistPic->height()-deltaHeight)));
            pa1->setEndValue(QSize(p.width(),p.height()));
            pa1->setDuration(500);
            pa1->start(QPropertyAnimation::DeleteWhenStopped);
        }
        {
            QPropertyAnimation* pa1=new QPropertyAnimation(ui->label_artistPic,"minimumSize");
            pa1->setStartValue(QSize(qMax(0,ui->label_artistPic->width()-deltaWidth),
                                     qMax(0,ui->label_artistPic->height()-deltaHeight)));
            pa1->setEndValue(QSize(p.width(),p.height()));
            pa1->setDuration(500);
            pa1->start(QPropertyAnimation::DeleteWhenStopped);
        }
    } else {
        ui->label_artistPic->setMinimumHeight(p.height());
        ui->label_artistPic->adjustSize();
        ui->frame_art->adjustSize();
        ui->label_description->adjustSize();
        ui->label_description->setFixedHeight(ui->label_description->height());
    }
    setBio(ui->label_description->text());
}

void HArtistContext::setBio(QString bio) {
    ui->label_description->setText(bio);
    ui->label_description->adjustSize();

//    if(s_showTime.msecsTo(QTime::currentTime())>110)
    {
        QPropertyAnimation* pa=new QPropertyAnimation(ui->label_description,"maximumHeight");
        pa->setStartValue(ui->label_description->height());
        pa->setEndValue(ui->label_description->heightForWidth(ui->label_description->width()-deltaWidth));
        pa->setDuration((ui->label_description->heightForWidth(ui->label_description->width()-deltaWidth)-ui->label_description->height())*2);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
    } /*else {
        ui->label_description->setMaximumHeight((ui->label_description->heightForWidth(ui->label_description->width()-deltaWidth)-ui->label_description->height())*2);
    }*/
}

void HArtistContext::setMePic(QImage& pic) {
    if(!isVisible()) return;
    if(pic.width()!=70) pic=pic.scaledToWidth(70,Qt::SmoothTransformation);
    ui->label_you->setPixmap(pic);
}

void HArtistContext::setListenerCount(int a) {
    s_listenerCountCache=a;
    updateCounts();
}

void HArtistContext::setUserPlayCount(int a) {
    s_userPlayCountCache=a;
    updateCounts();
}

void HArtistContext::setPlayCount(int a) {
    s_playCountCache=a;
    updateCounts();
}

void HArtistContext::updateCounts() {
    ui->label_playcount->setText("<B>"+QString::number(s_playCountCache)+"</B> plays by <B>"+QString::number(s_listenerCountCache)+"</B> users");
    ui->label_userplaycount->setText("<B>"+QString::number(s_userPlayCountCache)+"</B> plays in your library");
}

void HArtistContext::setAlbums(HAlbum* album) {
    if(s_loadedAlbums.contains(album)) return;
    s_loadedAlbums.push_back(album);
    {
        HAlbumBox* ab=HAlbumBox::getBox(*album);
        if(s_showTime.msecsTo(QTime::currentTime())>110) {
            ab->setFixedHeight(0);
            ab->adjustSize();
            QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
            pa->setStartValue(0);
            pa->setEndValue(174);
            pa->setDuration(500);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }
        ab->adjustSize();
        ui->widget_albums->layout()->addWidget(ab);
    }
//    if(i-s_albumLoadCount!=toLoad) {
//        ui->label_moreAlbums->hide();
    /*} else */{
        ui->label_moreAlbums->setText(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
    }

    ++s_albumLoadCount;
    s_albumsToLoad+=2;
}

void HArtistContext::setTracks(HTrack* track) {
    if(s_loadedTracks.contains(track)) return;
    s_loadedTracks.push_back(track);
//    int i;
    {
        HTrackBox* ab=HTrackBox::getBox(*track);
        ui->widget_tracks->layout()->addWidget(ab);
        if(s_showTime.msecsTo(QTime::currentTime())>110) {
            QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
            pa->setStartValue(0);
            pa->setEndValue(32);
            pa->setDuration(500);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            ab->adjustSize();
        }
    }
//    if(i-s_trackLoadCount!=toLoad) {
//        ui->label_moreTracks->hide();
    /*} else*/ {
        ui->label_moreTracks->setText(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
    }
    ++s_trackLoadCount;
    s_tracksToLoad+=2;
}

void HArtistContext::setTags(QList<HTag *> tags) {
    int i;
    s_tagsToLoad=qMax(s_tagLoadCount?s_tagLoadCount*2:4,s_tagsToLoad);
    for(i=s_tagLoadCount;i<tags.size()&&i-s_tagLoadCount<s_tagsToLoad;i++) {
        HTagBox* ab=HTagBox::getBox(*tags[i]);
        QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
        pa->setStartValue(0);
        pa->setEndValue(40);
        pa->setDuration(500);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
        ui->widget_tags->layout()->addWidget(ab);
    }
    if(i-s_tagLoadCount!=s_tagsToLoad) {
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

void HArtistContext::setSimilar(HArtist* similar) {
    if(s_loadedSimilar.contains(similar)) return;
    s_loadedSimilar.push_back(similar);
    {
        HArtistBox* ab=HArtistBox::getBox(*similar);
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
        ui->widget_similarArtists->layout()->addWidget(ab);
    }
//    if(i-s_similarLoadCount!=toLoad) {
//        ui->label_moreArtists->hide();
    /*} else*/ {
        ui->label_moreArtists->setText(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
    }
    s_similarLoadCount++;
    s_similarToLoad+=2;
}

void HArtistContext::setShouts(HShout* shouts) {
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
