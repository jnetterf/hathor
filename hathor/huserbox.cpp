#include "huserbox.h"
#include "ui_huserbox.h"

HUserBox::HUserBox(HUser &rep, QWidget *parent) :
    QWidget(parent),
    s_rep(rep),
    ui(new Ui::HUserBox)
{
    ui->setupUi(this);
    ui->label_pic->setPixmap(rep.getPic(HUser::Medium));
    ui->label_username->setText(rep.getUsername());
}

HUserBox::~HUserBox()
{
    delete ui;
}
