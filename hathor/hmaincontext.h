#ifndef HMAINCONTEXT_H
#define HMAINCONTEXT_H

#include "hbackground.h"
#include <QMainWindow>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QTimer>
#include <QLabel>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QFont>
#include <QXmlReader>
#include <QSettings>
#include "hartist.h"
#include <lastfm/ws.h>
#include <QApplication>
#include <QMutex>
#include "lastfmext.h"
#include "hgraphicsview.h"
#include <lastfm/Artist>
#include <QThread>
#include "htoolbar.h"

struct ArtistAvatar;

struct ArtistAvatarList {
    QList<ArtistAvatar*> _aa;
    bool iterateHide(int right,int down,int it,int hTop=-66966,int hBottom=-66966);
    bool iterateHide(int it=0);
};

class HGraphicsPixmapItem : public QObject, public QGraphicsPixmapItem { Q_OBJECT public: HGraphicsPixmapItem(QImage& a, QGraphicsTextItem* const b) : QGraphicsPixmapItem(QPixmap::fromImage(a),b) {} };

class ArtistLabel : public QGraphicsTextItem {
    Q_OBJECT
public:
    static QList<ArtistLabel*> _u_;
    static ArtistLabel* _cur_;
    HArtist& s_rep;
    bool s_vis;
    HGraphicsPixmapItem* s_px;
    ArtistLabel(HArtist& rep) : s_rep(rep),s_vis(1),s_px(0) {
        _u_.push_back(this);
        hideInfo();
    }
    ~ArtistLabel() {
        _u_.removeOne(this);
    }

public slots:
    void showInfo() {
        if(s_vis) return;
        s_vis=1;
        if(_cur_==this) return;
        _cur_=this;
        for(int i=0;i<ArtistLabel::_u_.size();i++) if(ArtistLabel::_u_[i]!=this) ArtistLabel::_u_[i]->hideInfo();
        setHtml("<html>"
                "<table bgcolor='lightgrey' border='0' width='919' ><tr>"
                "<td><B><font size='6' face='candara' color='black'>"+s_rep.getName()+"</font>");
        QPropertyAnimation* pa=new QPropertyAnimation(this,"opacity");
        pa->setStartValue(0.1);
        pa->setEndValue(1.0);
        pa->setDuration(400);
        pa->setEasingCurve(QEasingCurve::OutSine);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
        if(s_px) {
            s_px->show();
            s_rep.sendTagNames(this,"setTags");
        } else QTimer::singleShot(20,this,SLOT(requestInfo()));
    }
    void requestInfo() {
        if(!s_vis) return;
        **s_rep.sendPic(HArtist::Large,this,"setPic")=1;
        s_rep.sendTagNames(this,"setTags");
    }

    void hideInfo() {
        if(!s_vis) return;
        if(s_px) {
            s_px->deleteLater();
            s_px=0;
        }
        s_vis=0;
        setHtml("<html>"
                "<table border='0' width='919' ><tr>"
                "<td><font size='6' face='candara' color='white'>"+s_rep.getName()+"</font>");
        QPropertyAnimation* pa=new QPropertyAnimation(this,"opacity");
        pa->setStartValue(0.1);
        pa->setEndValue(1.0);
        pa->setDuration(600);
        pa->setEasingCurve(QEasingCurve::OutSine);
        pa->start(QAbstractAnimation::DeleteWhenStopped);
    }
    void setPic(QImage& p) {
        if(!s_vis) return;
        if(s_px) {
            s_px->show();
            return;
        }

        s_px=new HGraphicsPixmapItem(p,this);


        s_px->setX(923-p.width());
        s_px->setY(4);
        s_px->show();
        s_px->setParent(this);
    }
    void setTags(QStringList l) {
        if(!s_vis) return;
        setHtml("<html>"
                "<table bgcolor='lightgrey' border='0' width='919' ><tr>"
                "<td><B><font size='6' face='candara' color='black'>"+s_rep.getName()+"</font></B></table><br>"
                "<font size='2' face='candara' color='white'>"+
                l.join(", "));
    }

private:
    void mousePressEvent(QGraphicsSceneMouseEvent*) {
        emit showContext();
    }
signals:
    void showContext();
};

class ArtistAvatar : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
    Q_PROPERTY(qreal echoOpacity READ echoOpacity WRITE setEchoOpacity)
    static QMutex s_infoMutex;
    int PX,PY;
public:
    HArtist& s_rep;
public slots:
    void setEchoOpacity(qreal opacity){QGraphicsPixmapItem::setOpacity(opacity);}
    void showInfo() {
        if(!s_infoMutex.tryLock()) {
            QTimer::singleShot(0,this,SLOT(showInfo()));
            return;
        }
        if(!_ready) {s_infoMutex.unlock(); return; };
        if(_okCur==this) {s_infoMutex.unlock(); return; };
        if(_pendShow&&_activeId!=_id){_pendShow=0;{s_infoMutex.unlock(); return; };}
        if(_shown){s_infoMutex.unlock(); return; };
        _shown=1;
        if(_anims.size()){
            for(int i=0;i<_u_.size();i++) _u_[i]->hideInfo();
            QTimer::singleShot(0,this,SLOT(showInfo()));
            _pendShow=1;
            _activeId=_id;
            {s_infoMutex.unlock(); return; };
        }
        _okCur=this;

        PX=-300+boundingRect().width()/2;
        PY=-300+boundingRect().height();
        if(!iterateHide(-300,-300,0)) {
            hideInfo(0);
            PX=0;
            if(!iterateHide(300,-300,0)) {
                hideInfo(0);
                PY=0;
                if(!iterateHide(300,300,0)) {
                    hideInfo(0);
                    PX=-300+boundingRect().width()/2;
                    iterateHide(-300,300,0);
                }
            }
        }

        _shown=1;
        QGraphicsTextItem* tx=new QGraphicsTextItem;
        tx->setHtml("<font color=\"white\">"+s_rep.getName()+"</font>");
        tx->setFont(QFont("Candara",24,QFont::Bold));
        tx->setPos(x()+PX,y()+PY);
        _gi_.push_back(tx);
        QGraphicsRectItem* ri=_sc->addRect(x(),y(),boundingRect().width(),boundingRect().height(),QPen(QColor("grey")));
        ri->setZValue(-1);
        _gi_.push_back(ri);


        _sc->addItem(tx);
        QPropertyAnimation* anim;
        anim=new QPropertyAnimation(tx, "opacity");
        anim->setStartValue(0.05);
        anim->setEndValue(1.0);
        anim->setDuration(1000);
        anim->start();

        s_infoMutex.unlock();
        showInfo2();
    }
    void hideInfo(int tm=300) {
        _shown=0;
        untrap(tm);
        while(_gi_.size()){
            delete _gi_.takeFirst();
        }
    }
    void showInfo2() {
        if(!s_infoMutex.tryLock()) {
            QTimer::singleShot(0,this,SLOT(showInfo2()));
            return;
        }
        if(_okCur!=this) {s_infoMutex.unlock(); return; }
//        s_rep.sendBioShort(this,"setBio");
//        QString tBioShort = s_rep.getBioShort();

        if(_okCur!=this) {s_infoMutex.unlock(); return; }
        s_rep.sendTagNames(this,"setTn");

        if(_okCur!=this) {s_infoMutex.unlock(); return; }
        s_rep.sendBioShort(this,"setTx");

        if(_okCur!=this) {s_infoMutex.unlock(); return; }
        {s_infoMutex.unlock(); return; }
    }

public:
    ArtistAvatar( QGraphicsScene* sc, QString name, ArtistAvatarList up ) : s_rep(HArtist::get(name)){
        _sc=sc;
        _shown=0;
        _id=++_lastId;
        _pendShow=0;
        s_rep.getName()=name;
        _neighbours_[1]=up;
        foreach(ArtistAvatar* a,up._aa) a->_neighbours_[3]._aa.push_back(this);
        _u_.push_back(this);
    }

    virtual ~ArtistAvatar() {
        _u_.removeOne(this);
        _trap.removeOne(this);
        _hidden_.removeOne(this);
        if(_okCur==this) _okCur=0;
        _ready=0;
    }

    void addLeft(ArtistAvatar* left) {
        if(_neighbours_[0]._aa.contains(left)) return;
        _neighbours_[0]._aa.push_back(left);
        left->_neighbours_[2]._aa.push_back(this);
    }

    qreal echoOpacity() const { return QGraphicsPixmapItem::opacity();}
    bool iterateHide(int it=0) {
        if(!_trap.contains(this)){
            _trap.push_back(this);

            QPropertyAnimation* anim;
            _anims.push_back(anim=new QPropertyAnimation(this, "echoOpacity"));
            anim->setStartValue(1.0);
            anim->setEndValue(0.05);
            anim->setDuration(150+5*it);
            anim->start();
        } else return 1;

        _neighbours_[1].iterateHide(it+1);
        _neighbours_[3].iterateHide(it+1);
        _neighbours_[2].iterateHide(it+1);
        _neighbours_[0].iterateHide(it+1);
        return 1;
    }

    bool iterateHide(int right,int down,int it,int hTop=-66966,int hBottom=-66966) {
        bool ok=1;
        Q_UNUSED(ok);   //jic

        //        qDebug()<<"In at "<<right<<" "<<down;
        if(!right||!down){
            //            qDebug()<<"Stopping because of invalid position.";
            return 1;
        }
        if(!_trap.contains(this)){
            _trap.push_back(this);

            if(it){
                QPropertyAnimation* anim;
                _anims.push_back(anim=new QPropertyAnimation(this, "echoOpacity"));
                anim->setStartValue(1.0);
                anim->setEndValue(0.05);
                anim->setDuration(1000+300*it);
                anim->start();
            } else {
                QPropertyAnimation* anim;
                _anims.push_back(anim=new QPropertyAnimation(this, "echoOpacity"));
                anim->setStartValue(1.0);
                anim->setEndValue(0.5);
                anim->setDuration(1000+300*it);
                anim->start();
                hTop=boundingRect().top()+y();
                hBottom=boundingRect().bottom()+y();
            }
        }

        if(right>0) {
            if(!_neighbours_[2]._aa.size()){ok=0;}
            else ok=_neighbours_[2].iterateHide(qMax(right-this->boundingRect().width(),0.0),down,it+1,hTop,hBottom);
        }

        if(!ok){
            if(!_neighbours_[2]._aa.size()&&scenePos().x()<1079) ok=1;
            else {
//                qDebug()<<"Failing at right positive"<<s_rep.getName();
                return 0;
            }
        }

        if(right<0) {
            if(!_neighbours_[0]._aa.size())ok=0;
            else ok=_neighbours_[0].iterateHide(qMin(right+this->boundingRect().width(),0.0),down,it+1,hTop,hBottom);
        }

        if(!ok) {
//                        qDebug()<<"Failing at right neg"<<s_rep.getName();
            return 0;
        }

        ok=1;

        if(down>0) {
            //            if(!_neighbours_[3]._aa.size())ok=0;  <-- it's okay to hit the bottom
            ok=_neighbours_[3].iterateHide(right,qMax(down-this->boundingRect().height(),0.0),it+1,hTop,hBottom);
        } /*else if ((this->boundingRect().bottom()+y()<hBottom)&&(this->boundingRect().top()+y()<hTop)){
            if(!_neighbours_[3]._aa.size())ok=0;
            else ok=_neighbours_[3].iterateHide(1,1,it+1,hTop,hBottom);
        }*/

        if(!ok) {
            //            qDebug()<<"Failing at down pos"<<s_rep.getName();
            ok=1;   //it's actually ok
        }

        if(down<0) {
            if(!_neighbours_[1]._aa.size())ok=0;
            else ok=_neighbours_[1].iterateHide(right,qMin(down+this->boundingRect().height(),0.0),it+1,hTop,hBottom);
        } /*else if (((this->boundingRect().top()+y()>hTop)&&(this->boundingRect().bottom()+y()>hBottom))) {
            if(!_neighbours_[1]._aa.size())ok=0;
            else ok=_neighbours_[1].iterateHide(1,1,it+1,hTop,hBottom);
        }*/

        if(!ok) {
//                        qDebug()<<"Failing at down neg"<<s_rep.getName();
            return 0;
        }

        return 1;
    }

    static void untrap(int tm=300) {
        if(_trap.size()){
            while(_anims.size()){
                _anims[0]->stop(); delete _anims[0];
                _anims.removeFirst();
            }
            while(_trap.size()){
                QPropertyAnimation* anim=new QPropertyAnimation(_trap[0], "echoOpacity");
                anim->setStartValue(_trap[0]->opacity());
                anim->setEndValue(1.0);
                anim->setDuration(tm);
                anim->start(QAbstractAnimation::DeleteWhenStopped);
                _trap.removeFirst();
            }
        }
        if(_okCur) {
            _okCur->_pendShow=0;
            _okCur->_shown=0;
            while(_okCur->_gi_.size()){
                delete _okCur->_gi_.takeFirst();
            }
        }
        _ready=1;
        _activeId=-1;
//        _okCur=0;
        /*
        QGraphicsScene* _sc;
        QList<QGraphicsItem*> _gi_;
        int _id;
        static QList<ArtistAvatar*> _trap;
        static QList<ArtistAvatar*> _u_;
        static QList<QPropertyAnimation*> _anims;
        static int _activeId; static int _lastId;
        static ArtistAvatar* _okCur;
        bool _pendShow;
        QList<ArtistAvatar*> _hidden_;*/
    }

private slots:
    void mousePressEvent(QGraphicsSceneMouseEvent *event=0) {
        if(!event||event->button()==Qt::LeftButton) {

            if(_anims.size()){
                for(int i=0;i<_u_.size();i++) _u_[i]->hideInfo();
                QTimer::singleShot(500,this,SLOT(mousePressEvent()));
                _pendShow=1;
                _activeId=_id;
                return;
            }

            _ready=0;
            _okCur=this;
            iterateHide();
            QTimer::singleShot(0,this,SLOT(emitShowContext()));
        }
    }
    void emitShowContext() {
        emit showContext();
    }

    void setTx(QString txt) {
        if(_okCur!=this) return;
        QGraphicsTextItem* tx=new QGraphicsTextItem;
        tx->setHtml("<font color=\"white\">"+txt+"</font>");
        tx->setFont(QFont("Candara",10,QFont::Bold));
        tx->setPos(x()+PX,y()+PY+70);
        tx->setTextWidth(300);
        _gi_.push_back(tx);

        _sc->addItem(tx);

        {
            QPropertyAnimation* anim;
            anim=new QPropertyAnimation(tx, "opacity");
            anim->setStartValue(0.05);
            anim->setEndValue(1.0);
            anim->setDuration(1000);
            anim->start();
        }
        tx->setParent(this);
    }

    void setTn(QStringList txt) {
        if(_okCur!=this) return;
        if(txt.size()>2) {
            QGraphicsTextItem* tx=new QGraphicsTextItem;
            tx->setHtml("<font color=\"white\">Tags: "+txt[0]+", "+txt[1]+", "+txt[2]+"</font>");
            tx->setFont(QFont("Candara",10,QFont::Bold));
            tx->setPos(x()+PX,y()+PY+40);
            _gi_.push_back(tx);

            _sc->addItem(tx);
            QPropertyAnimation* anim;
            anim=new QPropertyAnimation(tx, "opacity");
            anim->setStartValue(0.05);
            anim->setEndValue(1.0);
            anim->setDuration(1000);
            anim->start();
            tx->setParent(this);
        }
    }

public:
    bool _shown;
    static bool _ready;
    //private:
public:
    QGraphicsScene* _sc;
    QList<QGraphicsItem*> _gi_;
    int _id;
    static QList<ArtistAvatar*> _trap;
    static QList<ArtistAvatar*> _u_;
    static QList<QPropertyAnimation*> _anims;
    static int _activeId; static int _lastId;
    static ArtistAvatar* _okCur;
    bool _pendShow;
    QList<ArtistAvatar*> _hidden_;

    ArtistAvatarList _neighbours_[4];

signals:
    void showContext();
};

class MagicScene : public QGraphicsScene {
    Q_OBJECT
public:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
        for(int i=0;i<items(event->scenePos()).size();i++){
            ArtistAvatar* aa=dynamic_cast<ArtistAvatar*>(items(event->scenePos())[i]);
            if(aa){
                aa->showInfo();
                QGraphicsScene::mouseMoveEvent(event);
                return;
            }

            ArtistLabel* al=dynamic_cast<ArtistLabel*>(items(event->scenePos())[i]);
            if(al){
                al->showInfo();
                QGraphicsScene::mouseMoveEvent(event);
                return;
            }
        }
        for(int i=0;i<ArtistAvatar::_u_.size();i++) ArtistAvatar::_u_[i]->hideInfo();
        for(int i=0;i<ArtistLabel::_u_.size();i++) ArtistLabel::_u_[i]->hideInfo();
        ArtistAvatar::_okCur=0;
        QGraphicsScene::mouseMoveEvent(event);
    }

};

class HMainContext : public HGraphicsView {
    Q_OBJECT
    MagicScene* sc;

    void leaveEvent(QEvent *e) {
        for(int i=0;i<ArtistAvatar::_u_.size();i++) ArtistAvatar::_u_[i]->hideInfo();
        ArtistAvatar::_okCur=0;
        HGraphicsView::leaveEvent(e);
    }

    void hideEvent(QHideEvent *event) {
        while(ArtistAvatar::_anims.size()) {
            delete ArtistAvatar::_anims.takeFirst();
        }
        if(background) background->closeMode();
        HGraphicsView::hideEvent(event);
    }

    void showEvent(QShowEvent *event) {
        if(background) background->openMode();
        HGraphicsView::showEvent(event);
    }

public:
    explicit HMainContext(QWidget *parent = 0);

signals:
    void showContext(HArtist&);

public slots:
    void setup();

private:
    HBackground* background;
};

#endif
