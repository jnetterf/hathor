#ifndef HUSERBOX_H
#define HUSERBOX_H

#include <QWidget>
#include "huser.h"

namespace Ui {
class HUserBox;
}

class HUserBox : public QWidget
{
    Q_OBJECT
    HUser& s_rep;

public:
    explicit HUserBox(HUser& rep, QWidget *parent = 0);
    ~HUserBox();

public slots:
    void lockGeometry() { setFixedSize(size()); }
    QSize minimumSizeHint() const { return sizeHint(); }
    void setPic(QPixmap &p);

private:
    Ui::HUserBox *ui;
};

#endif // HUSERBOX_H
