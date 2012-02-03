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
    static QHash<QString,HTagBox*> s_map;
public:
    static HTagBox* getBox(HTag& rep);
    ~HTagBox();
    
private:
    Ui::HTagBox *ui;
};

#endif // HTAGBOX_H
