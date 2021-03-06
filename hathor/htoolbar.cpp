#include "htoolbar.h"
#include "hloginwidget.h"
#include "ui_htoolbar.h"
#include "hmainwindow.h"
#include <QGraphicsColorizeEffect>
#include <QPropertyAnimation>
#include <QTimer>

HToolbar* HToolbar::_singleton=0;

HToolbar::HToolbar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HToolbar)
{
    ui->setupUi(this);
    setPlaybackStatus("Stopped");
    _singleton=this;

    connect(ui->toolButton_play,SIGNAL(clicked(bool)),this,SLOT(tryPlay(bool)));
    connect(ui->toolButton_next,SIGNAL(clicked()),this,SLOT(tryNext()));
    connect(ui->lineEdit_search,SIGNAL(textChanged(QString)),HMainWindow::singleton(),SLOT(search(QString)));
    connect(ui->label_status,SIGNAL(linkActivated(QString)),HMainWindow::singleton(),SLOT(showNowPlaying()));
    connect(ui->toolButton_backButton,SIGNAL(pressed()),HMainWindow::singleton(),SLOT(back()));
    connect(ui->toolButton_home,SIGNAL(pressed()),HMainWindow::singleton(),SLOT(home()));
    connect(ui->toolButton_config,SIGNAL(pressed()),HMainWindow::singleton(),SLOT(config()));
    connect(ui->toolButton_shuffle,SIGNAL(toggled(bool)),HPlayer::singleton(),SLOT(setShuffle(bool)));
    connect(HPlayer::singleton(),SIGNAL(stateChanged(HAbstractTrackInterface::State)),this,SLOT(onStateChanged(HAbstractTrackInterface::State)));
    connect(HPlayer::singleton(),SIGNAL(trackChanged(HTrack&)),this,SLOT(onTrackChanged(HTrack&)));
}

HToolbar::~HToolbar()
{
    delete ui;
}

QString HToolbar::message() {
    return ui->label_status->text();
}

void HToolbar::setMessage(QString msg) {
    s_message=msg;

    QGraphicsColorizeEffect* goe=new QGraphicsColorizeEffect;
    goe->setColor(Qt::white);
    ui->frame->setGraphicsEffect(goe);
    QPropertyAnimation* pa=new QPropertyAnimation(goe,"strength");
    pa->setStartValue(0.0);
    pa->setEndValue(1.0);
    pa->setDuration(400);
    pa->start(QPropertyAnimation::DeleteWhenStopped);
    QTimer::singleShot(400,this,SLOT(setMessage2()));
}

void HToolbar::setMessage2() {

    setMessageSimple(s_message);

    QGraphicsColorizeEffect* goe=new QGraphicsColorizeEffect;
    goe->setColor(Qt::white);
    ui->frame->setGraphicsEffect(goe);
    QPropertyAnimation* pa=new QPropertyAnimation(goe,"strength");
    pa->setStartValue(1.0);
    pa->setEndValue(0.0);
    pa->setDuration(400);
    pa->start(QPropertyAnimation::DeleteWhenStopped);
}

void HToolbar::setMessageSimple(QString msg) {
    ui->toolButton_backButton->hide();
    ui->toolButton_config->hide();
    ui->toolButton_play->hide();
//    ui->line->hide();
//    ui->line_2->hide();
//    ui->line_3->hide();
    ui->lineEdit_search->hide();
    ui->toolButton_next->hide();
    ui->toolButton_shuffle->hide();
    ui->label_status->show();
    ui->label_status->setText(msg);
}

void HToolbar::clearMessage() {
    QGraphicsColorizeEffect* goe=new QGraphicsColorizeEffect;
    goe->setColor(Qt::white);
    ui->frame->setGraphicsEffect(goe);
    QPropertyAnimation* pa=new QPropertyAnimation(goe,"strength");
    pa->setStartValue(0.0);
    pa->setEndValue(1.0);
    pa->setDuration(400);
    pa->start(QPropertyAnimation::DeleteWhenStopped);
    QTimer::singleShot(400,this,SLOT(clearMessage2()));
}

void HToolbar::clearMessage2() {
    ui->toolButton_backButton->show();
    ui->toolButton_config->show();
    ui->toolButton_play->show();
//    ui->line->show();
//    ui->line_2->show();
//    ui->line_3->show();
    ui->lineEdit_search->show();
    ui->toolButton_next->show();
    ui->toolButton_shuffle->show();
    ui->label_status->show();
    ui->label_status->setText(s_playback);

    QGraphicsColorizeEffect* goe=new QGraphicsColorizeEffect;
    goe->setColor(Qt::white);
    ui->frame->setGraphicsEffect(goe);
    QPropertyAnimation* pa=new QPropertyAnimation(goe,"strength");
    pa->setStartValue(1.0);
    pa->setEndValue(0.0);
    pa->setDuration(400);
    pa->start(QPropertyAnimation::DeleteWhenStopped);
}

void HToolbar::setPlaybackStatus(QString s) {
    if(ui->toolButton_play->isVisible()&&ui->label_status->text()!=s_message) {
        ui->label_status->setText(s);
    }
    s_playback=s;
}

void HToolbar::setBackEnabled(bool a) {
    ui->toolButton_backButton->setEnabled(a);
}

void HToolbar::setPlayEnabled(bool enabled) {
    ui->toolButton_play->setEnabled(enabled);
}

void HToolbar::setPlayChecked(bool checked) {
    ui->toolButton_play->setChecked(checked);
}

void HToolbar::setHomeEnabled(bool enabled) {
    ui->toolButton_home->setEnabled(enabled);
}

bool HToolbar::playChecked() {
    return ui->toolButton_play->isChecked();
}

void HToolbar::tryPlay(bool play) {
    if(play) HPlayer::singleton()->resume();
    else HPlayer::singleton()->pause();
}

void HToolbar::tryNext() {
    HPlayer::singleton()->playNext();
}

void HToolbar::onStateChanged(HAbstractTrackInterface::State s) {
    switch(s) {
    case HAbstractTrackInterface::Paused:
        setPlaybackStatus("Paused");
        setPlayChecked(false);
        setPlayEnabled(true);
        break;
    case HAbstractTrackInterface::Stopped:
        setPlaybackStatus("Stopped");
        setPlayChecked(false);
        setPlayEnabled(false);
        break;
    case HAbstractTrackInterface::Buffering:
        setPlaybackStatus("Buffering...");
        setPlayChecked(true);
        setPlayEnabled(true);
        break;
    case HAbstractTrackInterface::Playing:
        if(HPlayer::singleton()->currentTrack()) setPlaybackStatus("<A href=\"more\">"+HPlayer::singleton()->currentTrack()->getTrackName()+" by "+HPlayer::singleton()->currentTrack()->getArtistName()+"</A> ("+
                                                                   HPlayer::singleton()->getProviderName()+")");
        else setPlaybackStatus("Unknown state");
        setPlayChecked(true);
        setPlayEnabled(true);
        break;
    case HAbstractTrackInterface::Searching:
        setPlaybackStatus("Searching for track...");
        setPlayChecked(true);
        setPlayEnabled(true);
        break;
    }
}

void HToolbar::onTrackChanged(HTrack&) {

}
