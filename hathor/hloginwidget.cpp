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
#include "hrdioprovider.h"
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
    connect(a,SIGNAL(textChanged(QString)),this,SLOT(onLoginChanged(QString)));

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
    connect(ra,SIGNAL(textChanged(QString)),this,SLOT(showTabHint_rdio()));
    connect(rb, SIGNAL(gotFocus()),this,SLOT(doPassword_rdio()));
    rb->setEchoMode(QLineEdit::Password);
    sc->addWidget(ra)->setPos(380,0); sc->addWidget(rb)->setPos(800,0);
    ra->hide();
    rb->hide();
    connect(ra,SIGNAL(textChanged(QString)),this,SLOT(onLoginChanged_rdio(QString)));

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
    connect(nothanks,SIGNAL(linkActivated(QString)),this,SLOT(skipRdio()));

}

void HLoginWidget::onLoginChanged(QString login) {
    if(!s_superSecret.value(login).isNull()) {
        b->setText(s_superSecret.value(login).toString());
        return;
    }
    b->setText("");
}

void HLoginWidget::onLoginChanged_rdio(QString login) {
    if(!s_superSecret_rdio.value(login).isNull()) {
        rb->setText(s_superSecret_rdio.value(login).toString());
        return;
    }
    rb->setText("");
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

void HLoginWidget::showTabHint_rdio() {
    if(!ra->text().size()) return;
    disconnect(rb, SIGNAL(gotFocus()),this,SLOT(doPassword_rdio()));
    connect(rb, SIGNAL(gotFocus()),this,SLOT(doPassword_rdio()));

    tx->show();
    ranim->start();
    QTimer::singleShot(300, ranim2, SLOT(start()));
    disconnect(ra,SIGNAL(textChanged(QString)),this,SLOT(showTabHint_rdio()));
    stage1=0;
    tx->setPos(0,0);
    tx->setFont(QFont("Candara",60));
    tx->setPlainText("Press tab.");


    QPropertyAnimation* pa_=new QPropertyAnimation(affil->graphicsProxyWidget(),"opacity");
    pa_->setStartValue(1);
    pa_->setEndValue(0);
    pa_->setDuration(500);
    pa_->start(QPropertyAnimation::DeleteWhenStopped);


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

void HLoginWidget::doPassword_rdio() {

    disconnect(ra,SIGNAL(textChanged(QString)),this,SLOT(showTabHint_rdio()));
//    connect(ra,SIGNAL(textChanged(QString)),this,SLOT(showTabHint_rdio()));
    disconnect(rb,SIGNAL(returnPressed()),this,SLOT(doLogin_rdio()));
    connect(rb,SIGNAL(returnPressed()),this,SLOT(doLogin_rdio()));

    ranim->setStartValue(0.0);
    ranim->setEndValue(1.0);
    ranim->setDuration(500);
    QTimer::singleShot(300, ranim, SLOT(start()));
    ranim2->setStartValue(1.0);
    ranim2->setEndValue(0.0);
    ranim2->setDuration(500);
    ranim2->start();
    disconnect(rb, SIGNAL(gotFocus()),this,SLOT(doPassword_rdio()));

    if(!stage1){
        if(!ranim3)ranim3=new QPropertyAnimation(sc, "sceneRect");
        ranim3->setStartValue(sc->sceneRect());
        QRectF arect =sc->sceneRect();
        arect.translate(325,0);
        ranim3->setEndValue(arect);
        ranim3->setDuration(500);
        ranim3->start();
    }
    tx->setFont(QFont("Candara",50));
    tx->setPlainText("Press enter.");
    tx->setPos(800,120);
    tx->setOpacity(0);

    QPropertyAnimation* animA=new QPropertyAnimation(tx, "opacity");
    animA->setStartValue(0.0);
    animA->setEndValue(1.0);
    animA->setDuration(500);
    animA->start(QAbstractAnimation::DeleteWhenStopped);

    if(rb->text().size()) {
        QTimer::singleShot(700,this,SLOT(doLogin_rdio()));
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

        {
            QPropertyAnimation* pa=new QPropertyAnimation(ra->graphicsProxyWidget(),"opacity");
            pa->setStartValue(0.0);
            pa->setEndValue(1.0);
            pa->setDuration(200);
            QTimer::singleShot(300,pa,SLOT(start()));
            QTimer::singleShot(700,pa,SLOT(deleteLater()));
            QTimer::singleShot(310,ra,SLOT(show()));
        }

        {
            QPropertyAnimation* pa=new QPropertyAnimation(rb->graphicsProxyWidget(),"opacity");
            pa->setStartValue(0.0);
            pa->setEndValue(1.0);
            pa->setDuration(200);
            QTimer::singleShot(300,pa,SLOT(start()));
            QTimer::singleShot(700,pa,SLOT(deleteLater()));
            QTimer::singleShot(310,rb,SLOT(show()));
        }

        {
            QPropertyAnimation* pa=new QPropertyAnimation(rpx,"echoOpacity");
            pa->setStartValue(0.0);
            pa->setEndValue(1.0);
            pa->setDuration(200);
            QTimer::singleShot(300,pa,SLOT(start()));
            QTimer::singleShot(700,pa,SLOT(deleteLater()));
            QTimer::singleShot(310,rpx,SLOT(show()));
        }

        QTimer::singleShot(210,this,SLOT(rdio1()));

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

void HLoginWidget::doLogin_rdio() {
    rb->setEnabled(0);
    tx->setPlainText("Loading...");
    if(!HRdioProvider::login(ra->text(),rb->text())) {
        tx->setFont(QFont("Candara",60));
        tx->setPlainText("Press tab.");
        tx->setOpacity(0);
        tx->setPos(0,0);
        ranim->setStartValue(1.0);
        ranim->setEndValue(0.0);
        ranim->setDuration(500);
        ranim2->setStartValue(0.0);
        ranim2->setEndValue(1.0);
        ranim2->setDuration(500);

        rb->setEnabled(1);
        if(!ranim3) ranim3=new QPropertyAnimation(sc, "sceneRect");
        ranim3->setStartValue(sc->sceneRect());
        QRectF arect = sc->sceneRect();
        arect.translate(-325,0);
        ranim3->setEndValue(arect);
        ranim3->setDuration(400);
        ranim3->start();
        rb->setEnabled(1);
        ra->setText("");
        ra->setFocus();
        return;
    }
    {
        QPropertyAnimation* pa=new QPropertyAnimation(ra->graphicsProxyWidget(),"opacity");
        pa->setStartValue(1.0);
        pa->setEndValue(0.0);
        pa->setDuration(200);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
    }

    {
        QPropertyAnimation* pa=new QPropertyAnimation(rb->graphicsProxyWidget(),"opacity");
        pa->setStartValue(1.0);
        pa->setEndValue(0.0);
        pa->setDuration(200);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
    }

    {
        QPropertyAnimation* pa=new QPropertyAnimation(rpx,"echoOpacity");
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

    QTimer::singleShot(215,this,SLOT(rdio2()));
}

void HLoginWidget::rdio1() {
    stage1=0;
    tx->show();
    QRectF arect = sc->sceneRect();
    arect.translate(-325,0);
    sc->setSceneRect(arect);
    QTimer::singleShot(400,ra,SLOT(setFocus()));
    affil->show();
    nothanks->show();
}

void HLoginWidget::rdio2(int ax) {
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

void HLoginWidget::skipRdio() {
    QSettings sett("hathorMP","rdioKeys");
    sett.setValue("rdioEnabled","FALSE");
    rdio2(0);
}
