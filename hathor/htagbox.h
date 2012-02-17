#ifndef HTAGBOX_H
#define HTAGBOX_H

#include <QWidget>
#include "htag.h"
#include "hgrowingwidget.h"

namespace Ui {
class HTagBox;
}

class HTagBox : public HGrowingWidget
{
    Q_OBJECT
    HTag& s_rep;
    explicit HTagBox(HTag& rep, QWidget *parent = 0);
public:
    static HTagBox* getBox(HTag& rep);
    ~HTagBox();

public slots:
    void setSummary(QString);
    
private:
    Ui::HTagBox *ui;
};

#endif // HTAGBOX_H
