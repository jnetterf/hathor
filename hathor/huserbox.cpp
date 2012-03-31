#include "huserbox.h"
#include "ui_huserbox.h"

HUserBox::HUserBox(HUser &rep, QWidget *parent) :
    HGrowingWidget(parent),
    s_rep(rep),
    ui(new Ui::HUserBox)
{
    ui->setupUi(this);
    rep.sendPic(HUser::Medium,this,"setPic");
    ui->label_username->setText(rep.getUsername());
}

void HUserBox::setPic(QImage& pic) {
    if(!isVisible()) return;
    if(pic.width()!=70) pic=pic.scaledToWidth(70,Qt::SmoothTransformation);
    ui->label_pic->setPixmap(pic);
}

HUserBox::~HUserBox()
{
    delete ui;
}
