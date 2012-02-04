#ifndef HARTISTBOX_H
#define HARTISTBOX_H

#include <QWidget>
#include <QTime>
#include "hartist.h"
#include "hgrowingwidget.h"

namespace Ui {
class HArtistBox;
}

class HArtistBox : public HGrowingWidget
{
    Q_OBJECT
    HArtist& s_rep;

    explicit HArtistBox(HArtist& rep, QWidget *parent = 0);
    static QHash<QString,HArtistBox*> s_map;
    QTime s_showTime;

    void showEvent(QShowEvent *e) { s_showTime=QTime::currentTime(); QWidget::showEvent(e); }
public:
    static HArtistBox* getBox(HArtist& rep);
    ~HArtistBox();
    
public slots:
//    void continueLoading();
    void requestContext() { emit contextRequested(s_rep); }
    void setPic(QPixmap p);
    void setTagNames(QStringList tsl);
    void setBioShort(QString sh);
signals:
    void contextRequested(HArtist& rep);

private:
    Ui::HArtistBox *ui;
};

#endif // HARTISTBOX_H
