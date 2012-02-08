#include "huserbox.h"
#include "ui_huserbox.h"

HUserBox::HUserBox(HUser &rep, QWidget *parent) :
    QWidget(parent),
    s_rep(rep),
    ui(new Ui::HUserBox)
{
    ui->setupUi(this);
    rep.sendPic(HUser::Medium,this,"setPic");
    ui->label_username->setText(rep.getUsername());
}

void HUserBox::setPic(QPixmap pic) {
    ui->label_pic->setPixmap(pic.scaledToWidth(70,Qt::SmoothTransformation));
}

HUserBox::~HUserBox()
{
    delete ui;
}
