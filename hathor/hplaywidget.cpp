#include "hplaywidget.h"
#include "ui_hplaywidget.h"
#include "kfadewidgeteffect.h"
#include "habstractmusicinterface.h"
#include <QTimer>

HPlayWidget::HPlayWidget(HArtist &artist, QWidget *parent) :
    QWidget(parent),
    s_rep(&artist),
    ui(new Ui::HPlayWidget)
{
    ui->setupUi(this);
    ui->label_info->setText("<center><B>"+artist.getName()+"</B> (<A href=\"close\">close</A>)");

    doSetup();
}

HPlayWidget::HPlayWidget(HAlbum &album, QWidget *parent) :
    QWidget(parent),
    s_rep(&album),
    ui(new Ui::HPlayWidget)
{
    ui->setupUi(this);
    ui->label_info->setText("<center><B>"+album.getAlbumName()+" by "+album.getArtistName()+"</B> (<A href=\"close\">close</A>)");
    delete ui->button_morphStation; ui->button_morphStation=0;
    delete ui->button_similarArtists; ui->button_similarArtists=0;
    delete ui->button_topSongs; ui->button_topSongs=0;
    delete ui->commandLinkButton_6; ui->commandLinkButton_6=0;
    delete ui->frame_r; ui->frame_r=0;
    ui->button_topAlbums->setText("Album");

    ui->verticalLayout_3->deleteLater();
    ui->button_topAlbums->adjustSize();

    doSetup();
    adjustSize();
}

HPlayWidget::HPlayWidget(HTrack &track, QWidget *parent) :
    QWidget(parent),
    s_rep(&track),
    ui(new Ui::HPlayWidget)
{
    ui->setupUi(this);
    ui->label_info->setText("<center><B>"+track.getTrackName()+" by "+track.getArtistName()+"</B> (<A href=\"close\">close</A>)");
    ui->button_morphStation->hide();
    ui->button_similarArtists->setText("Similar Tracks");
    ui->button_topSongs->setText("Play");
    ui->commandLinkButton_6->hide();
    ui->button_topAlbums->hide();

    doSetup();
}

void HPlayWidget::doSetup() {
    if(ui->button_similarArtists) connect(ui->button_similarArtists,SIGNAL(clicked()),this,SLOT(similarArtists()));
    if(ui->button_morphStation) connect(ui->button_morphStation,SIGNAL(clicked()),this,SLOT(morphStation()));
    if(ui->button_topAlbums) connect(ui->button_topAlbums,SIGNAL(clicked()),this,SLOT(topAlbums()));
    if(ui->button_topSongs) connect(ui->button_topSongs,SIGNAL(clicked()),this,SLOT(topSongs()));
    if(ui->label_info) connect(ui->label_info,SIGNAL(linkActivated(QString)),this,SIGNAL(closed()));

    if(ui->frame_r) {
        ui->frame_r->hide();
        connect(ui->tb_r1,SIGNAL(clicked()),this,SLOT(play1()));
        connect(ui->tb_r2,SIGNAL(clicked()),this,SLOT(play2()));
        connect(ui->tb_r3,SIGNAL(clicked()),this,SLOT(play3()));
        connect(ui->tb_r4,SIGNAL(clicked()),this,SLOT(play4()));
        connect(ui->tb_r5,SIGNAL(clicked()),this,SLOT(play5()));
    }

    if(ui->frame_l) {
        ui->frame_l->hide();
        connect(ui->tb_l1,SIGNAL(clicked()),this,SLOT(play1()));
        connect(ui->tb_l2,SIGNAL(clicked()),this,SLOT(play2()));
        connect(ui->tb_l3,SIGNAL(clicked()),this,SLOT(play3()));
        connect(ui->tb_l4,SIGNAL(clicked()),this,SLOT(play4()));
        connect(ui->tb_l5,SIGNAL(clicked()),this,SLOT(play5()));
    }
}

void HPlayWidget::reset() {
    ui->frame_l->hide();
    if(ui->frame_r) ui->frame_r->hide();
    ui->verticalLayout_2->addWidget(ui->frame_l);
    if(ui->frame_r) ui->verticalLayout_3->addWidget(ui->frame_r);

    if(!dynamic_cast<HTrack*>(s_rep)&&!dynamic_cast<HAlbum*>(s_rep)) ui->button_morphStation->show();
    if(!dynamic_cast<HAlbum*>(s_rep)) ui->button_similarArtists->show();
    if(!dynamic_cast<HTrack*>(s_rep)) ui->button_topAlbums->show();
    if(!dynamic_cast<HAlbum*>(s_rep)) ui->button_topSongs->show();

    if(ui->frame_l) {
        ui->tb_l1->setText("5");
        ui->tb_l2->setText("10");
        ui->tb_l3->setText("20");
        ui->tb_l4->setText("50");
        ui->tb_l5->setText("All");
    }
    if(ui->frame_r) {
        ui->tb_r1->setText("5");
        ui->tb_r2->setText("10");
        ui->tb_r3->setText("20");
        ui->tb_r4->setText("50");
        ui->tb_r5->setText("All");
    }
}

void HPlayWidget::similarArtists() {
    s_state=SimilarArtists;
    KFadeWidgetEffect* kfe=new KFadeWidgetEffect(ui->frame);
    reset();
    ui->label_info->setText("<center><B>How many songs do you want to play?</B> (<A href=\"close\">close</A>)");
    ui->button_similarArtists->hide();
    if(ui->frame_r) ui->frame_r->show();
    if(ui->frame_r) ui->verticalLayout_3->insertWidget(0,ui->frame_r);
    ui->label_rType->setText("<B><center>Similar Artists");
    QTimer::singleShot(0,kfe,SLOT(start()));
}

void HPlayWidget::morphStation() {
    s_state=MorphStation;
    KFadeWidgetEffect* kfe=new KFadeWidgetEffect(ui->frame);
    reset();
    ui->label_info->setText("<center><B>How many songs do you want to play?</B> (<A href=\"close\">close</A>)");
    ui->button_morphStation->hide();
    if(ui->frame_r) ui->frame_r->show();
    ui->label_rType->setText("<B><center>Morphing Station");
    QTimer::singleShot(0,kfe,SLOT(start()));
}

void HPlayWidget::topAlbums() {
    HAlbum* album=dynamic_cast<HAlbum*>(s_rep);

    if(album) {
        if(ui->radioButton_play->isChecked()) HPlayer::singleton()->clear();
        QMetaObject::invokeMethod(HPlayer::singleton()->getStandardQueue(),"queue",Qt::QueuedConnection,Q_ARG(HAlbum*,album));
        emit closed();
        return;
    }

    s_state=TopAlbums;
    KFadeWidgetEffect* kfe=new KFadeWidgetEffect(ui->frame);
    reset();
    ui->label_info->setText("<center><B>How many albums do you want to play?</B> (<A href=\"close\">close</A>)");
    ui->button_topAlbums->hide();
    ui->frame_l->show();

    ui->tb_l1->setText("1");
    ui->tb_l2->setText("2");
    ui->tb_l3->setText("5");
    ui->tb_l4->setText("10");
    ui->tb_l5->setText("All");
    ui->label_lType->setText("<B><center>Top Albums");
    QTimer::singleShot(0,kfe,SLOT(start()));
}

void HPlayWidget::topSongs() {
    HTrack* track=dynamic_cast<HTrack*>(s_rep);

    if(track) {
        if(ui->radioButton_play->isChecked()) HPlayer::singleton()->clear();
        QMetaObject::invokeMethod(HPlayer::singleton()->getStandardQueue(),"queue",Qt::QueuedConnection,Q_ARG(HTrack*,track));
        emit closed();
        return;
    }

    s_state=TopSongs;
    KFadeWidgetEffect* kfe=new KFadeWidgetEffect(ui->frame);
    reset();
    ui->label_info->setText("<center><B>How many songs do you want to play?</B> (<A href=\"close\">close</A>)");
    ui->button_topSongs->hide();
    ui->frame_l->show();
    ui->verticalLayout_2->insertWidget(0,ui->frame_l);
    ui->label_lType->setText("<B><center>Top Songs");
    QTimer::singleShot(0,kfe,SLOT(start()));
}

void HPlayWidget::play1() {
    if(ui->radioButton_play->isChecked()) HPlayer::singleton()->clear();
    HArtist* art=dynamic_cast<HArtist*>(s_rep);
    HTrack* track=dynamic_cast<HTrack*>(s_rep);
    switch(s_state) {
    case TopSongs:
        if(art) art->sendTracks(HPlayer::singleton()->getStandardQueue(),"queue",5);
        break;
    case TopAlbums:
        if(art) art->sendAlbums(HPlayer::singleton()->getStandardQueue(),"queue",1);
        break;
    case SimilarArtists:
    case MorphStation:  //for now...
        s_echoSongCount=1;
        if(art) art->sendSimilar(this,"echo",5);
        if(track) track->sendSimilar(HPlayer::singleton()->getStandardQueue(),"queue",5);
        break;
    }
    emit closed();
}

void HPlayWidget::play2() {
    if(ui->radioButton_play->isChecked()) HPlayer::singleton()->clear();
    HArtist* art=dynamic_cast<HArtist*>(s_rep);
    HTrack* track=dynamic_cast<HTrack*>(s_rep);
    switch(s_state) {
    case TopSongs:
        if(art) art->sendTracks(HPlayer::singleton()->getStandardQueue(),"queue",10);
        break;
    case TopAlbums:
        if(art) art->sendAlbums(HPlayer::singleton()->getStandardQueue(),"queue",2);
        break;
    case SimilarArtists:
    case MorphStation:  //for now...
        s_echoSongCount=2;
        if(art) art->sendSimilar(this,"echo",5);
        if(track) track->sendSimilar(HPlayer::singleton()->getStandardQueue(),"queue",10);
        break;
    }
    emit closed();
}

void HPlayWidget::play3() {
    if(ui->radioButton_play->isChecked()) HPlayer::singleton()->clear();
    HArtist* art=dynamic_cast<HArtist*>(s_rep);
    HTrack* track=dynamic_cast<HTrack*>(s_rep);

    switch(s_state) {
    case TopSongs:
        if(art) art->sendTracks(HPlayer::singleton()->getStandardQueue(),"queue",20);
        break;
    case TopAlbums:
        if(art) art->sendAlbums(HPlayer::singleton()->getStandardQueue(),"queue",5);
        break;
    case SimilarArtists:
    case MorphStation:  //for now...
        s_echoSongCount=2;
        if(art) art->sendSimilar(this,"echo",10);
        if(track) track->sendSimilar(HPlayer::singleton()->getStandardQueue(),"queue",20);
        break;
    }
    emit closed();
}

void HPlayWidget::play4() {
    if(ui->radioButton_play->isChecked()) HPlayer::singleton()->clear();
    HArtist* art=dynamic_cast<HArtist*>(s_rep);
    HTrack* track=dynamic_cast<HTrack*>(s_rep);

    switch(s_state) {
    case TopSongs:
        if(art) art->sendTracks(HPlayer::singleton()->getStandardQueue(),"queue",50);
        break;
    case TopAlbums:
        if(art) art->sendAlbums(HPlayer::singleton()->getStandardQueue(),"queue",10);
        break;
    case SimilarArtists:
    case MorphStation:  //for now...
        s_echoSongCount=5;
        if(art) art->sendSimilar(this,"echo",10);
        if(track) track->sendSimilar(HPlayer::singleton()->getStandardQueue(),"queue",50);
        break;
    }
    emit closed();
}

void HPlayWidget::play5() {
    if(ui->radioButton_play->isChecked()) HPlayer::singleton()->clear();
    HArtist* art=dynamic_cast<HArtist*>(s_rep);
    HTrack* track=dynamic_cast<HTrack*>(s_rep);

    switch(s_state) {
    case TopSongs:
        if(art) art->sendTracks(HPlayer::singleton()->getStandardQueue(),"queue");
        break;
    case TopAlbums:
        if(art) art->sendAlbums(HPlayer::singleton()->getStandardQueue(),"queue");
        break;
    case SimilarArtists:
    case MorphStation:  //for now...
        s_echoSongCount=10;
        if(art) art->sendSimilar(this,"echo",20);
        if(track) track->sendSimilar(HPlayer::singleton()->getStandardQueue(),"queue");
        break;
    }
    emit closed();
}

void HPlayWidget::echo(HArtist *a) {
    a->sendTracks(HPlayer::singleton()->getStandardQueue(),"queue",s_echoSongCount);
}

HPlayWidget::~HPlayWidget()
{
    delete ui;
}
