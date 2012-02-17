#include "hshoutbox.h"
#include "ui_hshoutbox.h"
#include "huserbox.h"
#include <QDebug>

static QString sufix(int a) {
    switch(a) {
    case 1:
        return "st";
    case 2:
        return "nd";
    case 3:
        return "rd";
    default:
        return "th";
    }
}

HShoutBox::HShoutBox(HShout& rep, QWidget *parent) :
    QWidget(parent),
    s_rep(rep),
    ui(new Ui::HShoutBox)
{
    ui->setupUi(this);
    ui->label_user->setText(rep.getShouter().getUsername()+" writes...");
    QString sh=rep.getShout();
    int li=0;
    if(rep.getShout().contains("[")) {
        QString likes=rep.getShout();
        likes.remove(0,likes.lastIndexOf('[')+1);
        likes.truncate(likes.indexOf(']'));
        li=likes.toInt();
        if(li) sh.remove("["+QString::number(li)+"]");
    }
    ui->label_comments->setText(sh);
    ui->label_dateLikes->setText(rep.getDate()+(li?(", <B>"+QString::number(li)+sufix(li)+"</B> to like"):""));
    ui->label_comments->updateGeometry();
    ui->label_comments->adjustSize();
    adjustSize();
    setFixedHeight(minimumSizeHint().height());
    adjustSize();
    readjustPriorities();
}

void HShoutBox::setPic(QPixmap& pic) {
    if(isHidden()) return;
    if(pic.width()!=70) pic=pic.scaledToWidth(70,Qt::SmoothTransformation);
    ui->label_pic->setPixmap(pic);
}

HShoutBox::~HShoutBox()
{
    delete ui;
}
