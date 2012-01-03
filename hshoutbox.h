#ifndef HSHOUTBOX_H
#define HSHOUTBOX_H

#include <QWidget>
#include "hshout.h"
#include "hgrowingwidget.h"

namespace Ui {
class HShoutBox;
}

class HShoutBox : public HGrowingWidget
{
    Q_OBJECT
    
    HShout& s_rep;
public:
    explicit HShoutBox(HShout& rep, QWidget *parent = 0);
    ~HShoutBox();

private:
    Ui::HShoutBox *ui;
};

#endif // HSHOUTBOX_H
