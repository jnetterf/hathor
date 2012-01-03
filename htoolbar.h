#ifndef HTOOLBAR_H
#define HTOOLBAR_H

#include <QWidget>

namespace Ui {
class HToolbar;
}

class HToolbar : public QWidget
{
    Q_OBJECT
    QString s_message;
    QString s_playback;
    static HToolbar* _singleton;

public:
    static HToolbar* singleton() { return _singleton; }
    explicit HToolbar(QWidget *parent = 0);
    ~HToolbar();


signals:
    void backPressed();

public slots:
    void setMessage(QString);
    void setMessageSimple(QString);
    void clearMessage();

    void setMessage2();
    void clearMessage2();

    void setPlaybackStatus(QString s);

    void setBackEnabled(bool enabled);
    void setPlayEnabled(bool enabled);
    void setPlayChecked(bool checked);

    void tryPlay(bool play);
    void tryNext();
    
private:
    Ui::HToolbar *ui;
};

#endif // HTOOLBAR_H
