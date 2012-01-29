#ifndef HARTISTBOX_H
#define HARTISTBOX_H

#include <QWidget>
#include "hartist.h"
#include "hgrowingwidget.h"

namespace Ui {
class HArtistBox;
}

class HArtistBox : public HGrowingWidget
{
    Q_OBJECT
    HArtist& s_rep;

public:
    explicit HArtistBox(HArtist& rep, QWidget *parent = 0);
    ~HArtistBox();
    
public slots:
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
