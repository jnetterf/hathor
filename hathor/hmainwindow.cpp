#include "hmainwindow.h"
#include "hmaincontext.h"
#include "kfadewidgeteffect.h"
#include "hloginwidget.h"
#include "lastfm/ws.h"
#include <QSettings>
#include <QGraphicsOpacityEffect>
#include <QDesktopServices>
#include "ui_hmainwindow.h"
#include "hsearchcontext.h"
#include "hplayercontext.h"
#include "hartistcontext.h"
#include "htrackcontext.h"
#include "halbumcontext.h"
#include "hconfigcontext.h"
#include "hplugin.h"

HMainWindow* HMainWindow::s_singleton=0;

HMainWindow::HMainWindow() : ui(new Ui::HMainWindow), s_ge(0), s_getLoggingAllowed(0), s_mc(0) {
    QSettings vl("Nettek","log");
    vl.setValue("pluginDialog",false);
    Q_ASSERT(!s_singleton);
    lastfm::ws::ApiKey = "2930525005911a092c4ecea896316eab";
    lastfm::ws::SharedSecret = "abfb0f3af98ab1997d290eeb3e064e01";
    s_singleton=this;

    ui->setupUi(this);
    ui->toolbar->hide();
        show();
        HLoginWidget* l=new HLoginWidget;
        ui->widget->layout()->addWidget(l);
        connect(l,SIGNAL(showMainContext()),this,SLOT(setupMainContext()));
    qApp->setQuitOnLastWindowClosed(true);

    connect(HPlayer::singleton(),SIGNAL(cantFind()),this,SLOT(cantFind()));
}

void HMainWindow::keyPressEvent(QKeyEvent *e) {
    if(e->key()==Qt::Key_MediaPlay||e->key()==Qt::Key_MediaTogglePlayPause||e->key()==Qt::Key_MediaPause) {
        if (HToolbar::singleton()) HToolbar::singleton()->tryPlay(!HToolbar::singleton()->playChecked());
        e->accept();
    } else if(e->key()==Qt::Key_MediaStop) {
        if (HToolbar::singleton()) HToolbar::singleton()->tryPlay(0);
        e->accept();
    } else if(e->key()==Qt::Key_MediaNext) {
        if (HToolbar::singleton()) HToolbar::singleton()->tryNext();
        e->accept();
    }
}

void HMainWindow::hideEvent(QHideEvent *e) {
    HCachedInfo::save();
    QWidget::hideEvent(e);
}

void HMainWindow::setupMainContext() {
    HPlayer::singleton()->loadPlugins(ui->widget->layout());
    connect(HPlayer::singleton(),SIGNAL(doneLoadingPlugins()),this,SLOT(setupMainContext_2()));
}

void HMainWindow::setupMainContext_2() {

    if(!ui->toolbar->isVisible()) {
        QPropertyAnimation* pa= new QPropertyAnimation(ui->toolbar,"maximumHeight");
        pa->setStartValue(1);
        ui->toolbar->adjustSize();
        pa->setEndValue(ui->toolbar->height());
        pa->start(QAbstractAnimation::DeleteWhenStopped);
        ui->toolbar->show();
    }

    QSettings sett("Nettek","Hathor_global");
    if(sett.value("precached",0).toBool()) {
        ui->toolbar->setMessageSimple("<i><center>Loading...</center></i>");
    } else {
        ui->toolbar->setMessageSimple("<i><center>Loading...</center></i>");
    }
    s_mc=new HMainContext;
    connect(s_mc,SIGNAL(showContext(HArtist&)),this,SLOT(showContext(HArtist&)));
    ui->widget->layout()->addWidget(s_mc);
    s_mc->setup();
    if(ui->toolbar->message()=="<i><center>Loading...</center></i>"||ui->toolbar->message()=="<i><center>Loading...</center></i>") ui->toolbar->clearMessage();
    s_contextStack.push_back(s_curContext=s_mc);

    QSettings s_l("Nettek","logging");
    if(s_l.value("permission").isNull()||s_l.value("permission").toString()=="AskMeLater") {
        QTimer::singleShot(60000,this,SLOT(setGetLoggingAllowed()));
    }
}

void HMainWindow::showContext(HArtist& a) {
    setContext(HArtistContext::getContext(a));
}

void HMainWindow::showContext(HAlbum& a) {
    setContext(HAlbumContext::getContext(a));
}

void HMainWindow::showContext(HTrack& a) {
    setContext(HTrackContext::getContext(a));
}

void HMainWindow::setContext(QWidget *ac) {
    KFadeWidgetEffect* kfe=new KFadeWidgetEffect(ui->widget);
    if(s_curContext) {
        if(dynamic_cast<HSearchContext*>(s_curContext.data())) {
            HSearchContext::detach();
        }
        s_curContext->hide();
    }
    ui->gridLayout->addWidget(ac);
    s_contextStack.push_back(ac);
    ac->adjustSize();
    ui->toolbar->setBackEnabled(1);
    ui->toolbar->setHomeEnabled(1);
    s_curContext=ac;
    s_curContext->show();
    QTimer::singleShot(40,kfe,SLOT(start()));
}


void HMainWindow::back() {
    KFadeWidgetEffect* kwe=new KFadeWidgetEffect(ui->widget);
    if(s_curContext) s_curContext->hide();
    ui->gridLayout->removeWidget(s_contextStack.back());
    if(!dynamic_cast<HSearchContext*>(s_contextStack.back())&&!dynamic_cast<HPlayerContext*>(s_contextStack.back())) {
        s_contextStack.back()->deleteLater();
    }
    s_contextStack.pop_back();

    if(s_contextStack.size()==1) {
        ArtistAvatar::untrap();
    }

    ui->gridLayout->addWidget(s_contextStack.back());
    s_contextStack.back()->show();
    s_curContext=s_contextStack.back();

    ui->toolbar->setBackEnabled(s_contextStack.size()!=1);
    ui->toolbar->setHomeEnabled(s_contextStack.size()!=1);
    QTimer::singleShot(20,kwe,SLOT(start()));
}

void HMainWindow::home() {
    if(!s_contextStack.size()) return;
    while(s_contextStack.size()>2) s_contextStack.pop_back();
    back();
    ui->toolbar->setHomeEnabled(0);
}

void HMainWindow::config() {
    if(dynamic_cast<HConfigContext*>(s_curContext.data())) return;
    setContext(HConfigContext::singleton());
}


void HMainWindow::search(QString s) {
    if(s==""&&s_curContext==HSearchContext::singleton()) {
        back();
        return;
    }
    if(s_curContext!=HSearchContext::singleton()) setContext(HSearchContext::singleton());
    HSearchContext::singleton()->setSearchTerm(s);
}

void HMainWindow::ask(QString str, QObject *replyTo, QString replySlot) {
    if(!replyTo) return;
    if(s_curQuestion.replyTo) {
        s_question_Q.push_back(HQuestion(str,replyTo,replySlot));
    }
    else {
        s_curQuestion=HQuestion(str,replyTo,replySlot);
        activateQuestion();
    }
}

void HMainWindow::activateQuestion() {
    KFadeWidgetEffect* fwe=new KFadeWidgetEffect(ui->widget);
    ui->widget->setGraphicsEffect(s_ge=new QGraphicsBlurEffect);
    ui->widget->setEnabled(0);
    ui->toolbar->setEnabled(0);
    HQuestionWidget* qw=new HQuestionWidget(this,s_curQuestion.str);
    qw->adjustSize();
    if(qw->width()<100) qw->setMinimumWidth(100);
    if(qw->height()<20) qw->setMinimumWidth(20);
    qw->setGeometry(ui->widget->width()/2-qw->geometry().width()/2,ui->widget->height()/2-qw->geometry().height()/2,qw->geometry().width(),qw->geometry().height());
    qw->show();
    QGraphicsOpacityEffect* oe=new QGraphicsOpacityEffect;
    QPropertyAnimation* pa=new QPropertyAnimation(oe,"opacity");
    pa->setStartValue(0.0);
    pa->setEndValue(0.8);
    pa->setDuration(1000);
    pa->start(QAbstractAnimation::DeleteWhenStopped);
//    oe->setOpacity(0.8);
    qw->setGraphicsEffect(oe);
    connect(qw,SIGNAL(done(QString)),this,SLOT(hideAsk(QString)));
    fwe->start();
}


void HMainWindow::hideAsk(QString l) {
    QMetaObject::invokeMethod(s_curQuestion.replyTo,s_curQuestion.replySlot.toAscii(),Qt::QueuedConnection,Q_ARG(QString,l));

    KFadeWidgetEffect* fwe=new KFadeWidgetEffect(ui->widget);
    delete s_ge;
    s_ge=0;
    ui->widget->setEnabled(1);
    ui->toolbar->setEnabled(1);
    fwe->start();

    if(s_question_Q.size()) {
        s_curQuestion=s_question_Q.takeFirst();
        QTimer::singleShot(300,this,SLOT(activateQuestion()));
        return;
    }
    s_curQuestion=HQuestion();
}


void HMainWindow::showNowPlaying() {
    if(s_curContext!=HPlayerContext::singleton()) setContext(HPlayerContext::singleton());
}

void HMainWindow::setGetLoggingAllowed() {
    s_getLoggingAllowed=1;
}

void HMainWindow::getLogging() {
//    if(!s_getLoggingAllowed) return;
//    s_getLoggingAllowed=0;
//    ask("<center><B><font size='4'>Help make Hathor better!</B><br></font>Hathor logs crashes, page load times, and other boring information which can be used to improve Hathor.<br>"
//        "Can Hathor send this info off to its developper?<br><br><A href=\"no\">Configure Hathor</A>&nbsp;&nbsp;or&nbsp;&nbsp;<B><font size='4'><A href=\"yes\">Yes! I'm a good person!</A></B>",this,"setLogging");
}

void HMainWindow::setLogging(QString result) {
    if(result=="yet") {
        QSettings s_l("Nettek","logging");
        s_l.setValue("permission","yes");
    } else if(result=="no") {
        config();
    }
}

void HMainWindow::cantFind() {
    QSettings l("Nettek","log");
    if(!l.value("pluginDialog").toBool()) {
        l.setValue("pluginDialog",true);
        HMainWindow::singleton()->ask("<center><b><font size='4'>Sorry, Hathor couldn't find a song you wanted to listen to.</b></font><br>"
                                      "Go to <A href=\"http://hathor.nettek.ca/plugins\">hathor.nettek.ca/plugins</A> to get more plugins!<br>"
                                      "If you added music to your local collection, you may need to restart Hathor.<br><br><A href=\"null\">close</A>",
                                      HMainWindow::singleton(),"openUrl");
    }
}

void HMainWindow::openUrl(QString a) {
    if(a=="null") return;
    QDesktopServices::openUrl(QUrl(a));
}

void HMainWindow::setTitle(HTrack& a) {
    setWindowTitle(a.getTrackName()+" by "+a.getArtistName()+" - Hathor (beta)");
}

