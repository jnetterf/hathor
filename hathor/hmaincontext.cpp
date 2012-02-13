#include "hmaincontext.h"
#include <QList>
#include <QTimer>
#include <QDesktopWidget>
#include <QCompleter>
#include "hartistcontext.h"
#include "halbumcontext.h"
#include "htrackcontext.h"
#include <lastfm/ws.h>
#include <lastfm/misc.h>
#include <lastfm/XmlQuery>
#include <QGraphicsColorizeEffect>
#include <QDesktopServices>
#include <QPalette>
#include "lastfmext.h"
#include "hsearchcontext.h"
#include "hplayercontext.h"
#include "kfadewidgeteffect.h"

QList<ArtistLabel*> ArtistLabel::_u_;
ArtistLabel* ArtistLabel::_cur_=0;

QList<ArtistAvatar*> ArtistAvatar::_trap;
QList<QPropertyAnimation*> ArtistAvatar::_anims;
QList<ArtistAvatar*> ArtistAvatar::_u_;
int ArtistAvatar::_activeId=-1; int ArtistAvatar::_lastId=-1;
ArtistAvatar* ArtistAvatar::_okCur;
QMutex ArtistAvatar::s_infoMutex;

bool ArtistAvatarList::iterateHide(int right,int down,int it,int hTop,int hBottom) {
    bool ok=0;
    foreach(ArtistAvatar*a,_aa) {
        if(a->iterateHide(right,down,it,hTop,hBottom)) ok=1;
    }
    return ok;
}

bool ArtistAvatarList::iterateHide(int it) {
    foreach(ArtistAvatar*a,_aa) {
        a->iterateHide(it);
    }
    return 1;
}

HMainContext::HMainContext(QWidget *parent) : HGraphicsView(parent), background(0) {
    setBackgroundRole(QPalette::Base);
    sc = new MagicScene;
    setMouseTracking(1);
    setScene(sc);
    setFrameStyle(QFrame::NoFrame);
}

void HMainContext::setup() {
    show();
    setResizeAnchor(QGraphicsView::NoAnchor);
    setSceneRect(400,-300,600,8000);
    verticalScrollBar()->setValue(-300);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setMax(10000);
    background = new HBackground(sc);   // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    connect(background,SIGNAL(showContext(HArtist&)),this,SIGNAL(showContext(HArtist&)));
}
