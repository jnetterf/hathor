#ifndef HMAINWINDOW_H
#define HMAINWINDOW_H

#include <QWidget>
#include <QList>

namespace Ui {
class HMainWindow;
}

class HArtist;
class HAlbum;
class HTrack;

class HMainWindow : public QWidget
{
    Q_OBJECT
    Ui::HMainWindow *ui;
    QWidget* s_curContext;
    QList<QWidget*> s_contextStack;
    static HMainWindow* s_singleton;
public:
    HMainWindow();
    static HMainWindow* singleton() { return s_singleton; }
    void keyPressEvent(QKeyEvent * e);

public slots:
    void setupMainContext();
    void showContext(HArtist& a);
    void showContext(HAlbum& a);
    void showContext(HTrack& a);
    void setContext(QWidget *ac);
    void back();
    void home();
    void search(QString s);
    void showNowPlaying();

    void setTitle(HTrack& a);
};

#endif // HMAINWINDOW_H
