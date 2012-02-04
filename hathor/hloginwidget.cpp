#include "hloginwidget.h"
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

HLoginWidget::HLoginWidget(QWidget *parent):HGraphicsView(parent) {
    setMinimumHeight(768);
    setMinimumWidth(1000);
    if(qApp->desktop()->width()<1224) {
        setWindowState(Qt::WindowFullScreen);
    }
    tx=0;
    setBackgroundRole(QPalette::Base);
    setWindowTitle("Hathor");
    sc = new QGraphicsScene;

    stage1=1;

    QTimer::singleShot(0,this,SLOT(continueLoading()));
}

void HLoginWidget::continueLoading() {
    QSettings auth("hathorMP","auth");
    if(auth.value("lfm.username").isValid()&&auth.value("lfm.key").isValid()) {
        lastfm::ws::Username = auth.value("lfm.username").toString();
        lastfm::ws::SessionKey = auth.value("lfm.key").toString();
        hide();
        emit showMainContext();
        deleteLater();
        return;
    }
    ///
    px = new FadePixmap;
    px->setPixmap(QPixmap(":/icons/lfmRed.jpg").scaledToHeight(100,Qt::SmoothTransformation));
    px->setPos(0,0);
    sc->addItem(px);
    anim=new QPropertyAnimation(px, "echoOpacity");
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->setDuration(1000);
    ///
    tx = new QGraphicsTextItem;
    tx->setFont(QFont("Candara",60));
    tx->setPlainText("Press tab.");
    tx->setOpacity(0);
    sc->addItem(tx);
    anim2=new QPropertyAnimation(tx, "opacity");
    anim2->setStartValue(0.0);
    anim2->setEndValue(1.0);
    anim2->setDuration(1000);
    ///
    anim3=0;
    ///
    a=new MagicLineEdit;b=new MagicLineEdit;
    a->resize(400, 100);b->resize(400,100);
    a->setFont(QFont("Arial",60));b->setFont(QFont("Arial",60));
    a->setPlaceholderText("username"); b->setPlaceholderText("password");
    connect(a,SIGNAL(textChanged(QString)),this,SLOT(showTabHint()));
    connect(b, SIGNAL(gotFocus()),this,SLOT(doPassword()));
    b->setEchoMode(QLineEdit::Password);
    sc->addWidget(a)->setPos(380,0); sc->addWidget(b)->setPos(800,0);
    ///
    sc->setSceneRect(-10,-100,780,300);
    setScene(sc);

    QTimer::singleShot(200,a,SLOT(setFocus()));

    QSettings settings("hathorMP","lastfm_ext");
    s_superSecret=settings.value("keys").toMap();
    s_superSecret_rdio=settings.value("keys_rdio").toMap();

    ///
    rpx = new FadePixmap;
    rpx->setPixmap(QPixmap(":/icons/rdio.png").scaledToHeight(100,Qt::SmoothTransformation));
    rpx->setPos(000,0);
    rpx->hide();
    sc->addItem(rpx);
    ranim=new QPropertyAnimation(rpx, "echoOpacity");
    ranim->setStartValue(1.0);
    ranim->setEndValue(0.0);
    ranim->setDuration(1000);
    ///
    ranim2=new QPropertyAnimation(tx, "opacity");
    ranim2->setStartValue(0.0);
    ranim2->setEndValue(1.0);
    ranim2->setDuration(1000);
    ///
    ranim3=0;
    ///
    ra=new MagicLineEdit;rb=new MagicLineEdit;
    ra->resize(400, 100);rb->resize(400,100);
    ra->setFont(QFont("Arial",60));rb->setFont(QFont("Arial",60));
    ra->setPlaceholderText("email"); rb->setPlaceholderText("password");
    rb->setEchoMode(QLineEdit::Password);
    sc->addWidget(ra)->setPos(380,0); sc->addWidget(rb)->setPos(800,0);
    ra->hide();
    rb->hide();

    affil=new QLabel;
    affil->setText("<a href=\"http://click.linksynergy.com/fs-bin/click?id=EtH0bqD6seI&offerid=221756.10000004&type=3&subid=0\" >"
                   "Sign up free!</a>");
    sc->addWidget(affil)->setPos(380,120);
    affil->hide();
    affil->setPalette(QPalette(Qt::white));
    affil->setTextInteractionFlags(Qt::TextBrowserInteraction);
    affil->setFont(QFont("Candara",30));
    affil->adjustSize();
    connect(affil,SIGNAL(linkActivated(QString)),this,SLOT(openLink(QString)));

    nothanks=new QLabel;
    nothanks->setText("<a href=\"NO_THANKS\" >Skip!</a>");
    sc->addWidget(nothanks)->setPos(698,120);
    nothanks->hide();
    nothanks->setPalette(QPalette(Qt::white));
    nothanks->setTextInteractionFlags(Qt::TextBrowserInteraction);
    nothanks->setFont(QFont("Candara",30));
    nothanks->adjustSize();
}

void HLoginWidget::showTabHint() {
    if(!a->text().size()) return;
    disconnect(b, SIGNAL(gotFocus()),this,SLOT(doPassword()));
    connect(b, SIGNAL(gotFocus()),this,SLOT(doPassword()));

    anim->start();
    QTimer::singleShot(300, anim2, SLOT(start()));
    disconnect(a,SIGNAL(textChanged(QString)),this,SLOT(showTabHint()));
    stage1=0;
}

void HLoginWidget::doPassword() {
    disconnect(b, SIGNAL(gotFocus()),this,SLOT(doPassword()));
    disconnect(a,SIGNAL(textChanged(QString)),this,SLOT(showTabHint()));

//    connect(a,SIGNAL(textChanged(QString)),this,SLOT(showTabHint()));

    disconnect(b,SIGNAL(returnPressed()),this,SLOT(doLogin()));
    connect(b,SIGNAL(returnPressed()),this,SLOT(doLogin()));

    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setDuration(500);
    QTimer::singleShot(300, anim, SLOT(start()));
    anim2->setStartValue(1.0);
    anim2->setEndValue(0.0);
    anim2->setDuration(500);
    anim2->start();
    disconnect(b, SIGNAL(gotFocus()),this,SLOT(doPassword()));

    if(!stage1){
        if(!anim3)anim3=new QPropertyAnimation(sc, "sceneRect");
        anim3->setStartValue(sc->sceneRect());
        QRectF arect =sc->sceneRect();
        arect.translate(325,0);
        anim3->setEndValue(arect);
        anim3->setDuration(500);
        anim3->start();
    }
    if(!tx) {
        tx = new QGraphicsTextItem;
        sc->addItem(tx);
    }
    tx->setFont(QFont("Candara",50));
    tx->setPlainText("Press enter.");
    tx->setOpacity(0);
    tx->setPos(800,120);

    QPropertyAnimation* animA=new QPropertyAnimation(tx, "opacity");
    animA->setStartValue(0.0);
    animA->setEndValue(1.0);
    animA->setDuration(500);
    animA->start(QAbstractAnimation::DeleteWhenStopped);

    if(b->text().size()) {
        QTimer::singleShot(700,this,SLOT(doLogin()));
        b->setEnabled(0);
    }
}

void HLoginWidget::doLogin() {
    b->setEnabled(0);

    lastfm::ws::Username = a->text();
    QString password = b->text();

    QMap<QString, QString> params;
    params["method"] = "auth.getMobileSession";
    params["username"] = lastfm::ws::Username;
    params["authToken"] = lastfm::md5( (lastfm::ws::Username + lastfm::md5( password.toUtf8() )).toUtf8() );
    QNetworkReply* reply = lastfmext_post( params );

    QEventLoop loop;
    loop.connect( reply, SIGNAL(finished()), SLOT(quit()) );
    loop.exec();

    try {
        QSettings auth("hathorMP","auth");
        lastfm::XmlQuery const lfm = lastfm::ws::parse( reply );
        lastfm::ws::Username = lfm["session"]["name"].text();
        lastfm::ws::SessionKey = lfm["session"]["key"].text();

        auth.setValue("lfm.username",lfm["session"]["name"].text());
        auth.setValue("lfm.key",lfm["session"]["key"].text());

        {
            QPropertyAnimation* pa=new QPropertyAnimation(a->graphicsProxyWidget(),"opacity");
            pa->setStartValue(1.0);
            pa->setEndValue(0.0);
            pa->setDuration(200);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }

        {
            QPropertyAnimation* pa=new QPropertyAnimation(b->graphicsProxyWidget(),"opacity");
            pa->setStartValue(1.0);
            pa->setEndValue(0.0);
            pa->setDuration(200);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }

        {
            QPropertyAnimation* pa=new QPropertyAnimation(px,"echoOpacity");
            pa->setStartValue(1.0);
            pa->setEndValue(0.0);
            pa->setDuration(200);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }

        {
            QPropertyAnimation* pa=new QPropertyAnimation(tx,"opacity");
            pa->setStartValue(1.0);
            pa->setEndValue(0.0);
            pa->setDuration(200);
            pa->start(QAbstractAnimation::DeleteWhenStopped);
        }

        // RDIO


        QTimer::singleShot(210,this,SLOT(finish()));

        HUser::get(lastfm::ws::Username).getPic(HUser::Medium); //CACHE
        tx->setOpacity(0);

    } catch (std::runtime_error& e) {
        tx->setFont(QFont("Candara",60));
        tx->setPlainText("Press tab.");
        tx->setOpacity(0);
        tx->setPos(0,0);
        anim->setStartValue(1.0);
        anim->setEndValue(0.0);
        anim->setDuration(500);
        anim2->setStartValue(0.0);
        anim2->setEndValue(1.0);
        anim2->setDuration(500);

        b->setEnabled(1);
        if(!anim3) anim3=new QPropertyAnimation(sc, "sceneRect");
        anim3->setStartValue(sc->sceneRect());
        QRectF arect = sc->sceneRect();
        arect.translate(-325,0);
        anim3->setEndValue(arect);
        anim3->setDuration(400);
        anim3->start();
        b->setEnabled(1);
        b->setText("");
        s_superSecret.insert(a->text(),"");
        a->setText("");
        a->setFocus();


        return;
    }
}

void HLoginWidget::finish(int ax) {
    show();
    QRectF arect = sc->sceneRect();
    arect.translate(ax,800);
    sc->setSceneRect(arect);

    QSettings auth("hathorMP","auth");
    auth.setValue("lfm.username",lastfm::ws::Username);
    auth.setValue("lfm.key",lastfm::ws::SessionKey);

    QSettings settings("hathorMP","lastfm_ext");
    s_superSecret.insert(a->text(),b->text());
    settings.setValue("keys",s_superSecret);

    hide();
    emit showMainContext();
    deleteLater();
}

void HLoginWidget::openLink(QString s) {
    QDesktopServices::openUrl(QUrl(s));
}

