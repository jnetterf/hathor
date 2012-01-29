#include "hplaywidget.h"
#include "ui_hplaywidget.h"
#include "kfadewidgeteffect.h"
#include "habstractmusicinterface.h"
#include <QTimer>

HArtistPlayWidget::HArtistPlayWidget(HArtist &artist, QWidget *parent) :
    QWidget(parent),
    s_artist(artist),
    ui(new Ui::HPlayWidget)
{
    ui->setupUi(this);
    ui->label_info->setText("<center><B>"+artist.getName()+"</B> (<A href=\"close\">close</A>)");

    connect(ui->button_similarArtists,SIGNAL(clicked()),this,SLOT(similarArtists()));
    connect(ui->button_morphStation,SIGNAL(clicked()),this,SLOT(morphStation()));
    connect(ui->button_topAlbums,SIGNAL(clicked()),this,SLOT(topAlbums()));
    connect(ui->button_topSongs,SIGNAL(clicked()),this,SLOT(topSongs()));
    connect(ui->label_info,SIGNAL(linkActivated(QString)),this,SIGNAL(closed()));
    connect(ui->label_info,SIGNAL(linkActivated(QString)),this,SLOT(deleteLater()));
    ui->frame_l->hide();
    ui->frame_r->hide();

    connect(ui->tb_l1,SIGNAL(clicked()),this,SLOT(play1()));
    connect(ui->tb_r1,SIGNAL(clicked()),this,SLOT(play1()));
    connect(ui->tb_l2,SIGNAL(clicked()),this,SLOT(play2()));
    connect(ui->tb_r2,SIGNAL(clicked()),this,SLOT(play2()));
    connect(ui->tb_l3,SIGNAL(clicked()),this,SLOT(play3()));
    connect(ui->tb_r3,SIGNAL(clicked()),this,SLOT(play3()));
    connect(ui->tb_l4,SIGNAL(celicked()),this,SLOT(play4()));
    connect(ui->tb_r4,SIGNAL(clicked()),this,SLOT(play4()));
    connect(ui->tb_l5,SIGNAL(clicked()),this,SLOT(play5()));
    connect(ui->tb_r5,SIGNAL(clicked()),this,SLOT(play5()));
}

void HArtistPlayWidget::reset() {
    ui->frame_l->hide();
    ui->frame_r->hide();
    ui->verticalLayout_2->addWidget(ui->frame_l);
    ui->verticalLayout_3->addWidget(ui->frame_r);

    ui->button_morphStation->show();
    ui->button_similarArtists->show();
    ui->button_topAlbums->show();
    ui->button_topSongs->show();

    ui->tb_l1->setText("5");
    ui->tb_l2->setText("10");
    ui->tb_l3->setText("20");
    ui->tb_l4->setText("50");
    ui->tb_l5->setText("All");
    ui->tb_r1->setText("5");
    ui->tb_r2->setText("10");
    ui->tb_r3->setText("20");
    ui->tb_r4->setText("50");
    ui->tb_r5->setText("All");
}

void HArtistPlayWidget::similarArtists() {
    s_state=SimilarArtists;
    KFadeWidgetEffect* kfe=new KFadeWidgetEffect(ui->frame);
    reset();
    ui->label_info->setText("<center><B>How many songs do you want to play?</B> (<A href=\"close\">close</A>)");
    ui->button_similarArtists->hide();
    ui->frame_r->show();
    ui->verticalLayout_3->insertWidget(0,ui->frame_r);
    ui->label_rType->setText("<B><center>Similar Artists");
    QTimer::singleShot(0,kfe,SLOT(start()));
}

void HArtistPlayWidget::morphStation() {
    s_state=MorphStation;
    KFadeWidgetEffect* kfe=new KFadeWidgetEffect(ui->frame);
    reset();
    ui->label_info->setText("<center><B>How many songs do you want to play?</B> (<A href=\"close\">close</A>)");
    ui->button_morphStation->hide();
    ui->frame_r->show();
    ui->label_rType->setText("<B><center>Morphing Station");
    QTimer::singleShot(0,kfe,SLOT(start()));
}

void HArtistPlayWidget::topAlbums() {
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

void HArtistPlayWidget::topSongs() {
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

void HArtistPlayWidget::play1() {
    if(ui->radioButton_play->isChecked()) HPlayer::singleton()->clear();
    switch(s_state) {
    case TopSongs:
        s_artist.sendTracks(HPlayer::singleton()->getStandardQueue(),"queue",5);
        break;
    case TopAlbums:
        s_artist.sendAlbums(HPlayer::singleton()->getStandardQueue(),"queue",1);
        break;
    case SimilarArtists:
    case MorphStation:  //for now...
        s_echoSongCount=1;
        s_artist.sendSimilar(this,"echo",5);
        break;
    }
    emit closed();
}

void HArtistPlayWidget::play2() {
    if(ui->radioButton_play->isChecked()) HPlayer::singleton()->clear();
    switch(s_state) {
    case TopSongs:
        s_artist.sendTracks(HPlayer::singleton()->getStandardQueue(),"queue",10);
        break;
    case TopAlbums:
        s_artist.sendAlbums(HPlayer::singleton()->getStandardQueue(),"queue",2);
        break;
    case SimilarArtists:
    case MorphStation:  //for now...
        s_echoSongCount=2;
        s_artist.sendSimilar(this,"echo",5);
        break;
    }
    emit closed();
}

void HArtistPlayWidget::play3() {
    if(ui->radioButton_play->isChecked()) HPlayer::singleton()->clear();

    switch(s_state) {
    case TopSongs:
        s_artist.sendTracks(HPlayer::singleton()->getStandardQueue(),"queue",20);
        break;
    case TopAlbums:
        s_artist.sendAlbums(HPlayer::singleton()->getStandardQueue(),"queue",5);
        break;
    case SimilarArtists:
    case MorphStation:  //for now...
        s_echoSongCount=2;
        s_artist.sendSimilar(this,"echo",10);
        break;
    }
    emit closed();
}

void HArtistPlayWidget::play4() {
    if(ui->radioButton_play->isChecked()) HPlayer::singleton()->clear();

    switch(s_state) {
    case TopSongs:
        s_artist.sendTracks(HPlayer::singleton()->getStandardQueue(),"queue",50);
        break;
    case TopAlbums:
        s_artist.sendAlbums(HPlayer::singleton()->getStandardQueue(),"queue",10);
        break;
    case SimilarArtists:
    case MorphStation:  //for now...
        s_echoSongCount=5;
        s_artist.sendSimilar(this,"echo",10);
        break;
    }
    emit closed();
}

void HArtistPlayWidget::play5() {
    if(ui->radioButton_play->isChecked()) HPlayer::singleton()->clear();

    switch(s_state) {
    case TopSongs:
        s_artist.sendTracks(HPlayer::singleton()->getStandardQueue(),"queue");
        break;
    case TopAlbums:
        s_artist.sendAlbums(HPlayer::singleton()->getStandardQueue(),"queue");
        break;
    case SimilarArtists:
    case MorphStation:  //for now...
        s_echoSongCount=10;
        s_artist.sendSimilar(this,"echo",20);
        break;
    }
    emit closed();
}

void HArtistPlayWidget::echo(HArtist *a) {
    a->sendTracks(HPlayer::singleton()->getStandardQueue(),"queue",s_echoSongCount);
}

HArtistPlayWidget::~HArtistPlayWidget()
{
    delete ui;
}
