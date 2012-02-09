#include "htrackcontext.h"
#include "ui_htrackcontext.h"
#include "htrackbox.h"
#include "htrackbox.h"
#include "htagbox.h"
#include "hshoutbox.h"
#include "hartistbox.h"
#include "halbumbox.h"
#include "habstractmusicinterface.h"
#include <QRect>
#include <QScrollBar>
#include <QMenu>
#include <QIcon>
#include "kfadewidgeteffect.h"
#include "hnettloger.h"
#include <QGraphicsBlurEffect>

QHash<QString, HTrackContext*> HTrackContext::s_map;

HTrackContext* HTrackContext::getContext(HTrack &rep) {
    QString dumbName=rep.getTrackName()+"__"+rep.getArtistName();
    if(s_map.contains(dumbName)) return s_map[dumbName];
    s_map[dumbName] = new HTrackContext(rep);
    return s_map[dumbName];
}

HTrackContext::HTrackContext(HTrack& rep, QWidget *parent) :
    QWidget(parent),
    s_rep(rep),
    s_albumLoadCount(0),
    s_artistLoadCount(0),
    s_tagLoadCount(0),
    s_similarLoadCount(0),
    s_shoutLoadCount(0),
    s_similarToLoad(10),
    s_loved(0),
    s_contentSet(0),
    s_pw(0),
    s_ge(0),
    s_slideshow(0),
    ui(new Ui::HTrackContext)
{
    s_showTime=QTime::currentTime();

    ui->setupUi(this);
    s_priorities[0].push_back(s_rep.sendSummary(this,"setContent"));
    s_priorities[1].push_back(s_rep.sendPlayCount(this,"setPlayCount"));
    s_priorities[1].push_back(s_rep.sendListenerCount(this,"setListenerCount"));
    s_priorities[1].push_back(s_rep.sendUserPlayCount(this,"setUserPlayCount"));

    ui->label_track->setText(s_rep.getTrackName());

    connect(ui->label_moreDescription,SIGNAL(linkActivated(QString)),this,SLOT(showMoreBio()));

    connect(ui->button_play,SIGNAL(clicked()),this,SLOT(play()));
    connect(ui->toolButton_loved,SIGNAL(clicked()),this,SLOT(toggleLoved()));
//    connect(ui->button_more,SIGNAL(clicked()),this,SLOT(playSimilar()));

    ui->widget_artist->setLayout(new QVBoxLayout);
    ui->widget_albums->setLayout(new QVBoxLayout);
    ui->widget_tags->setLayout(new QVBoxLayout);
    ui->widget_similar->setLayout(new QVBoxLayout);
    ui->widget_comments->setLayout(new QVBoxLayout);

    loadArtist();
    loadAlbum();
    loadSimilar();
    loadShouts();
    loadTags();

    connect(ui->label_moreTracks,SIGNAL(linkActivated(QString)),this,SLOT(loadSimilar()));

    connect(ui->label_moreTags,SIGNAL(linkActivated(QString)),this,SLOT(loadTags()));

    connect(ui->label_moreShoutbox,SIGNAL(linkActivated(QString)),this,SLOT(loadShouts()));

    HUser::get(lastfm::ws::Username).sendPic(HUser::Medium,this,"setMePic");

    ui->frame_header->adjustSize();

    // menus
//    QMenu* playMenu=new QMenu;
//    playMenu->addAction("Queue",this,SLOT(playTrack()));
//    playMenu->addAction("Replace",this,SLOT(playReplacing()));
//    ui->button_play->setMenu(playMenu);

//    QMenu* moreMenu=new QMenu;
//    moreMenu->addAction("Queue five similar tracks",this,SLOT(playSimilar()));
//    moreMenu->addAction("Queue ten similar tracks",this,SLOT(playMoreSimilar()));
//    moreMenu->addAction("Replace queue with ten songs",this,SLOT(playMoreSimilarReplacing()));
//    ui->button_more->setMenu(moreMenu);

    s_priorities[3].push_back(s_rep.sendBpm(this,"setBpm"));
    s_priorities[3].push_back(s_rep.sendValence(this,"setValence"));
    s_priorities[3].push_back(s_rep.sendAggression(this,"setAggression"));
    s_priorities[3].push_back(s_rep.sendAvgLoudness(this,"setAvgLoudness"));
    s_priorities[3].push_back(s_rep.sendPercussiveness(this,"setPercussiveness"));
    s_priorities[3].push_back(s_rep.sendKey(this,"setKey"));
    s_priorities[3].push_back(s_rep.sendEnergy(this,"setEnergy"));
    s_priorities[3].push_back(s_rep.sendPunch(this,"setPunch"));
    s_priorities[3].push_back(s_rep.sendSoundCreativity(this,"setSoundCreativity"));
    s_priorities[3].push_back(s_rep.sendChordalClarity(this,"setChordalClarity"));
    s_priorities[3].push_back(s_rep.sendTempoInstability(this,"setTempoInstability"));
    s_priorities[3].push_back(s_rep.sendRhythmicIntricacy(this,"setRhythmicIntricacy"));
    s_priorities[3].push_back(s_rep.sendSpeed(this,"setSpeed"));

    s_priorities[0].push_back(s_rep.sendLoved(this,"setLoved"));
    readjustPriorities();
}

HTrackContext::~HTrackContext()
{
    delete ui;
}

void HTrackContext::showEvent(QShowEvent *e) {
    HL("HTrackContext::showEvent: "+s_rep.getTrackName()+" "+s_rep.getArtistName());
    s_showTime=QTime::currentTime();
    //our boxes may have been stolen while we weren't looking >_<

    while(ui->widget_artist->layout()->count()) {
        ui->widget_artist->layout()->removeItem(ui->widget_artist->layout()->itemAt(0));
    }
    while(ui->widget_tags->layout()->count()) {
        ui->widget_tags->layout()->removeItem(ui->widget_tags->layout()->itemAt(0));
    }
    while(ui->widget_albums->layout()->count()) {
        ui->widget_albums->layout()->removeItem(ui->widget_albums->layout()->itemAt(0));
    }
    while(ui->widget_similar->layout()->count()) {
        ui->widget_similar->layout()->removeItem(ui->widget_similar->layout()->itemAt(0));
    }

    s_loadedSimilar.clear();

    s_artistLoadCount=0;
    s_tagLoadCount=0;
    s_albumLoadCount=0;

    loadArtist();
    loadTags();
    loadAlbum();
    loadSimilar(s_similarLoadCount?s_similarLoadCount:s_similarToLoad);

    readjustPriorities();
    QWidget::showEvent(e);
}

void HTrackContext::hideEvent(QHideEvent *e) {
    readjustPriorities();
    QWidget::hideEvent(e);
}

void HTrackContext::showMoreBio()
{
    ui->label_moreDescription->setText("Loading...");
    s_priorities[1].push_back(s_rep.sendContent(this,"setContent"));
    readjustPriorities();
}

void HTrackContext::setMePic(QPixmap pic) {
    ui->label_you->setPixmap(pic.scaledToWidth(70,Qt::SmoothTransformation));
}

void HTrackContext::loadArtist()
{
    if(!isVisible()) return;
    ui->label_moreArtists->setText("<p align=\"right\"><i>Loading...</i></p>");
    {
        HArtistBox* ab=HArtistBox::getBox(s_rep.getArtist());
        if(s_showTime.msecsTo(QTime::currentTime())>110) {
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

void HTrackContext::loadAlbum()
{
    ui->label_moreAlbums->setText("<p align=\"right\"><i>Loading...</i></p>");
    s_priorities[0].push_back(s_rep.sendAlbums(this,"setAlbums"));
    readjustPriorities();
}

void HTrackContext::loadTags()
{
    ui->label_moreTags->setText("<p align=\"right\"><i>Loading...</i></p>");
    if(s_tagLoadCount) s_priorities[3].push_back(s_rep.sendMoreTags(this,"setTags"));
    else s_priorities[3].push_back(s_rep.sendTags(this,"setTags"));
    readjustPriorities();
}

void HTrackContext::loadShouts()
{
    ui->label_moreShoutbox->setText("<p align=\"right\"><i>Loading...</i></p>");
//    s_priorities[3].push_back(s_rep.sendShouts(this,"setShouts"));
//    readjustPriorities();
}

void HTrackContext::loadSimilar(int s)
{   
    ui->label_moreArtists->setText("<p align=\"right\"><i>Loading...</i></p>");
    if(s==-1) s_priorities[1].push_back(s_rep.sendSimilar(this,"setSimilar",s_similarToLoad));
    else {
        s_similarLoadCount=0;
        s_similarToLoad=4;
        s_priorities[1].push_back(s_rep.sendSimilar(this,"setSimilar",s));
    }
    readjustPriorities();
}

//void HTrackContext::play() {
//    HPlayer::singleton()->clear();
//    HPlayer::singleton()->getStandardQueue()->queue(&s_rep);
//}

void HTrackContext::play() {
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

void HTrackContext::hidePlay() {
    KFadeWidgetEffect* fwe=new KFadeWidgetEffect(this);
    delete s_ge;
    s_ge=0;
    s_pw->hide();
    setEnabled(1);
    fwe->start();

}

void HTrackContext::setContent(QString t) {
    ui->label_description->setText(t.size()?t:"No article is available for this track.");
    ui->label_description->adjustSize();

    if(s_contentSet) ui->label_moreDescription->hide();
    s_contentSet=1;
}

void HTrackContext::setPlayCount(int t) {
    s_playCountCache=t;
    updateBoxes();
}

void HTrackContext::setListenerCount(int t) {
    s_listenerCountCache=t;
    updateBoxes();
}

void HTrackContext::updateBoxes() {
    ui->label_playcount->setText("<B>"+QString::number(s_playCountCache)+"</B> plays by <B>"+QString::number(s_listenerCountCache)+"</B> users");
}

void HTrackContext::setUserPlayCount(int t) {
    ui->label_userplaycount->setText("<B>"+QString::number(t)+"</B> plays in your library");
}

void HTrackContext::setAlbums(QList<HAlbum *> t) {
    if(!isVisible()) return;
    ui->label_moreAlbums->hide();
    if(t.size())
    {
        HAlbumBox* ab=HAlbumBox::getBox(*t[0]);
        if(s_showTime.msecsTo(QTime::currentTime())>110) {
            QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
            pa->setStartValue(0);
            pa->setEndValue(ab->sizeHint().height());
            pa->setDuration(500);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }
        ui->widget_albums->layout()->addWidget(ab);
    }
    s_albumLoadCount=1;
    ui->label_moreArtists->hide();
}

void HTrackContext::setTags(QList<HTag *> tags) {
    if(!isVisible()) return;
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

void HTrackContext::setShouts(QList<HShout *> shouts) {
    int i;
    int toLoad=s_shoutLoadCount?s_shoutLoadCount*2:10;
    for(i=s_shoutLoadCount;i<shouts.size()&&i-s_shoutLoadCount<toLoad;i++) {
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

void HTrackContext::setSimilar(HTrack* similar) {
    if(!isVisible()) return;
    if(!similar) {
        ui->label_moreTracks->hide();
        return;
    }
    ui->label_moreTracks->setText(
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
        "p, li { white-space: pre-wrap; }"
        "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
        "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
        "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
    if(s_loadedSimilar.contains(similar)) return;
    s_loadedSimilar.push_back(similar);
    {
        HTrackBox* ab=HTrackBox::getBox(*similar);
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
        ui->widget_similar->layout()->addWidget(ab);
    }
//    if(i-s_similarLoadCount!=toLoad) {
//        ui->label_moreArtists->hide();
    /*} else*/ {
    }
    s_similarLoadCount++;
    s_similarToLoad+=2;




//    int i;
//    int toLoad=s_similarLoadCount?s_similarLoadCount*2:4;
//    for(i=s_similarLoadCount;i<tracks.size()&&i-s_similarLoadCount<toLoad;i++) {
//        HTrackBox* ab=HTrackBox::getBox(*tracks[i]);
//        if(s_showTime.msecsTo(QTime::currentTime())>110) {
//            QPropertyAnimation* pa=new QPropertyAnimation(ab,"maximumHeight");
//            pa->setStartValue(0);
//            pa->setEndValue(40);
//            pa->setDuration(500);
//            pa->start(QAbstractAnimation::DeleteWhenStopped);
//        }
//        ui->widget_similar->layout()->addWidget(ab);
//    }
//    if(i-s_similarLoadCount!=toLoad) {
//        ui->label_moreTracks->hide();
//    } else {
//        ui->label_moreTracks->setText(
//            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> "
//            "p, li { white-space: pre-wrap; }"
//            "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">"
//            "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"a1\">"
//            "<span style=\" text-decoration: underline; color:#0057ae;\">more...</span></a></p></body></html>");
//    }

//    s_similarLoadCount=i;
}

void HTrackContext::setSlideshow(QWidget *w) {
    if(!w) {
        s_slideshow->hide();
        s_slideshow=0;
        return;
    }
    s_slideshow=w;
    ui->content->addWidget(w);
    ui->content->setAlignment(w,Qt::AlignCenter);
    ui->scrollArea_3->hide();
    w->adjustSize();
    ui->frame_header->adjustSize();
    w->show();
}

void HTrackContext::setLoved(bool a) {
    if(a) {
        ui->toolButton_loved->setIcon(QIcon(":/icons/heart-red.png"));
    }
}

void HTrackContext::toggleLoved() {
    HL("HTrackContext::toggleLoved: "+s_rep.getTrackName()+" by "+s_rep.getArtistName()+" TO "+(s_loved?"unloved":"loved"));

    QMap<QString, QString> params;
    params["method"] = s_loved?"track.unlove":"track.love";
    params["artist"] = s_rep.getArtistName();
    params["track"] = s_rep.getTrackName();

    QNetworkReply* reply = lastfmext_post( params );
    setLoved(!s_loved);//FIXME::REPLACE
}

void HTrackContext::setBpm(int d) {
    s_bpm=d;
    if(s_bpm) {
        ui->label->setText(s_character+QString(s_character.size()?", ":"")+"<B>"+QString::number(s_bpm)+"</B> bpm");
    } else {
        ui->label->setText("Last.fm has not yet analysed this song...");
    }
}

void HTrackContext::setValence(double d) {
    if(d<-0.3) s_character+=QString(s_character.size()?", ":" ")+"moody";
    if(d>-0.01) s_character+=QString(s_character.size()?", ":" ")+"happy";
}

void HTrackContext::setAggression(double d) {
    if(d>-0.985) s_character+=QString(s_character.size()?", ":" ")+"aggressive";
    if(d<-0.99) s_character+=QString(s_character.size()?", ":" ")+"smooth";
}

void HTrackContext::setAvgLoudness(double d) {
    if(d>-16.5) s_character+=QString(s_character.size()?", ":" ")+"loud";
}

void HTrackContext::setPercussiveness(double d) {
    if(d>0.65) s_character+=QString(s_character.size()?", ":" ")+"percussive";
    setBpm(s_bpm);
}

void HTrackContext::setKey(int z) {

}

void HTrackContext::setEnergy(double d) {
    if(d>0.7) s_character+=QString(s_character.size()?", ":" ")+"energetic";
    if(d<0.35) s_character+=QString(s_character.size()?", ":" ")+"calm";
    setBpm(s_bpm);
}

void HTrackContext::setPunch(double d) {
    if(d>0.45) s_character+=QString(s_character.size()?", ":" ")+"punchy";
    if(d<0.2) s_character+=QString(s_character.size()?", ":" ")+"not punchy";
}

void HTrackContext::setSoundCreativity(double d) {
//        qDebug()<<d<<"(loudness)";

    if(d>0.55) s_character+=QString(s_character.size()?", ":" ")+"interesting";
    if(d<0.4) s_character+=QString(s_character.size()?", ":" ")+"conventional";

}

void HTrackContext::setChordalClarity(double d) {
    if(d>0.6) s_character+=QString(s_character.size()?", ":" ")+"clear";
    if(d<0.4) s_character+=QString(s_character.size()?", ":" ")+"not music";

}

void HTrackContext::setTempoInstability(double d) {
}

void HTrackContext::setRhythmicIntricacy(double d) {
    if(d>0.6) s_character+=QString(s_character.size()?", ":" ")+"rhymic";
    if(d<0.3) s_character+=QString(s_character.size()?", ":" ")+"not rhymic";
}

void HTrackContext::setSpeed(double d) {

}
