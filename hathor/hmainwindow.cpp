#include "hmainwindow.h"
#include "hmaincontext.h"
#include "kfadewidgeteffect.h"
#include "hloginwidget.h"
#include "hrdioprovider.h"
#include "lastfm/ws.h"
#include <QSettings>
#include "ui_hmainwindow.h"
#include "hsearchcontext.h"
#include "hplayercontext.h"
#include "hartistcontext.h"
#include "htrackcontext.h"
#include "halbumcontext.h"

HMainWindow* HMainWindow::s_singleton=0;

HMainWindow::HMainWindow() : ui(new Ui::HMainWindow) {
    Q_ASSERT(!s_singleton);
    lastfm::ws::ApiKey = "2930525005911a092c4ecea896316eab";
    lastfm::ws::SharedSecret = "abfb0f3af98ab1997d290eeb3e064e01";
    s_singleton=this;

    ui->setupUi(this);
    ui->toolbar->hide();
    QSettings sett("hathorMP","rdioKeys");
    if(sett.value("rdioEnabled").toString()=="FALSE"||HRdioProvider::restore()) {
        QSettings auth("hathorMP","auth");

        lastfm::ws::Username = auth.value("lfm.username").toString();
        lastfm::ws::SessionKey = auth.value("lfm.key").toString();
        show();
        ui->toolbar->show();
        setupMainContext();
        return;
    } else {
        show();
        ui->toolbar->hide();
        HLoginWidget* l=new HLoginWidget;
        ui->widget->layout()->addWidget(l);
        connect(l,SIGNAL(showMainContext()),this,SLOT(setupMainContext()));
    }
    qApp->setQuitOnLastWindowClosed(true);
}

void HMainWindow::setupMainContext() {
    if(ui->toolbar->isHidden()) {
        QPropertyAnimation* pa= new QPropertyAnimation(ui->toolbar,"maximumHeight");
        pa->setStartValue(1);
        pa->setEndValue(ui->toolbar->height());
        pa->start(QAbstractAnimation::DeleteWhenStopped);
        ui->toolbar->show();
    }

    QSettings sett("hathorMP","global");
    if(sett.value("precached",0).toBool()) {
        ui->toolbar->setMessageSimple("<i><center>Loading...</center></i>");
    } else {
        ui->toolbar->setMessageSimple("<i><center>Loading...</center></i>");
    }
    HMainContext* mc=new HMainContext;
    connect(mc,SIGNAL(showContext(HArtist&)),this,SLOT(showContext(HArtist&)));
    ui->widget->layout()->addWidget(mc);
    mc->setup();
    if(ui->toolbar->message()=="<i><center>Loading...</center></i>"||ui->toolbar->message()=="<i><center>Loading...</center></i>") ui->toolbar->clearMessage();
    s_contextStack.push_back(s_curContext=mc);
}

void HMainWindow::showContext(HArtist& a) {
    setContext(new HArtistContext(a));
}

void HMainWindow::showContext(HAlbum& a) {
    setContext(new HAlbumContext(a));
}

void HMainWindow::showContext(HTrack& a) {
    setContext(new HTrackContext(a));
}

void HMainWindow::setContext(QWidget *ac) {
    KFadeWidgetEffect* kwe=new KFadeWidgetEffect(ui->widget);
    if(s_curContext) {
        if(dynamic_cast<HSearchContext*>(s_curContext)) {
            HSearchContext::detach();
        }
        s_curContext->hide();
    }
    ui->gridLayout->addWidget(ac);
    s_contextStack.push_back(ac);
    ac->adjustSize();
    ui->toolbar->setBackEnabled(1);
    s_curContext=ac;
    s_curContext->show();
    kwe->start(300);
}


void HMainWindow::back() {
    KFadeWidgetEffect* kwe=new KFadeWidgetEffect(ui->widget);
    if(s_curContext) s_curContext->hide();
    s_contextStack.pop_back();

    if(s_contextStack.size()==1) {
        ArtistAvatar::untrap();
    }

    s_contextStack.back()->show();
    s_curContext=s_contextStack.back();

    ui->toolbar->setBackEnabled(s_contextStack.size()!=1);
    kwe->start( (s_contextStack.size()==1) ?300:100);
}


void HMainWindow::search(QString s) {
    if(s==""&&s_curContext==HSearchContext::singleton()) {
        back();
        return;
    }
    if(s_curContext!=HSearchContext::singleton()) setContext(HSearchContext::singleton());
    HSearchContext::singleton()->setSearchTerm(s);
}

void HMainWindow::showNowPlaying() {
    if(s_curContext!=HPlayerContext::singleton()) setContext(HPlayerContext::singleton());
}

void HMainWindow::setTitle(HTrack& a) {
    setWindowTitle(a.getTrackName()+" by "+a.getArtistName()+" - Hathor (beta)");
}

