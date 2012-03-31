#ifndef HMAINWINDOW_H
#define HMAINWINDOW_H

#include <QWidget>
#include <QList>
#include <QPointer>
#include <QFrame>
#include <QBoxLayout>
#include <QLabel>
#include <QDebug>

namespace Ui {
class HMainWindow;
}

class HArtist;
class HAlbum;
class HTrack;
class QGraphicsBlurEffect;

struct HQuestion {
    QString str;
    QObject* replyTo;
    QString replySlot;
    HQuestion(QString c,QObject* rt,QString rs) : str(c), replyTo(rt), replySlot(rs) {}
    HQuestion() : replyTo(0) {}
};

class HQuestionWidget : public QFrame {
    Q_OBJECT
public:
    HQuestionWidget(QWidget* p,QString q) : QFrame(p) {
        setLayout(new QVBoxLayout);
        QLabel* lab=new QLabel(q);
        layout()->addWidget(lab);
        setFrameStyle(QFrame::Panel|QFrame::Sunken);
        connect(lab,SIGNAL(linkActivated(QString)),this,SIGNAL(done(QString)));
        connect(lab,SIGNAL(linkActivated(QString)),this,SLOT(deleteLater()));
    }
signals:
    void done(QString);
};

class HMainContext;

class HMainWindow : public QWidget
{
    Q_OBJECT
    Ui::HMainWindow *ui;
    QPointer<QWidget> s_curContext;
    QList<QWidget*> s_contextStack;
    static HMainWindow* s_singleton;
    HQuestion s_curQuestion;
    QList<HQuestion> s_question_Q;
    QGraphicsBlurEffect* s_ge;
    bool s_getLoggingAllowed;
    HMainContext* s_mc;
public:
    HMainWindow();
    static HMainWindow* singleton() { return s_singleton; }
    void keyPressEvent(QKeyEvent * e);
    void hideEvent(QHideEvent *);

public slots:
    void setupMainContext();
    void setupMainContext_2();
    void showContext(HArtist& a);
    void showContext(HAlbum& a);
    void showContext(HTrack& a);
    void setContext(QWidget *ac);
    void back();
    void home();
    void config();
    void search(QString s);
    void ask(QString string,QObject* replyTo,QString replySlot);   //i.e., "Is it okay if we send data in order to improve Hathor?<br><A href="YES">YES</A>", options, object, "onResponse"
                                                                                       //replySlot should accept a string.
    void activateQuestion();
    void hideAsk(QString l);
    void showNowPlaying();

    void setGetLoggingAllowed();
    void getLogging();
    void setLogging(QString);

    void cantFind();

    void openUrl(QString);

    void setTitle(HTrack& a);
    void printLyrics(QString a) { qDebug()<<"LYRICS"<<a; }

};

#endif // HMAINWINDOW_H
