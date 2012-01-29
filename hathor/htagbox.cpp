#include "htagbox.h"
#include "ui_htagbox.h"

HTagBox::HTagBox(HTag &rep, QWidget *parent) :
    HGrowingWidget(parent),
    s_rep(rep),
    ui(new Ui::HTagBox)
{
    ui->setupUi(this);
    ui->label_tag->setText("<A href=\"more\">"+rep.getTagName()+"</A>");
//    ui->label_reachAndTaggings->setText("Built by <B>"+QString::number(rep.getTaggings())+"</B> and used <B>"+QString::number(rep.getReach())+"</B> times!");
    QString sum=rep.getSummary();
    sum.truncate(120);
    ui->label_desc->setText(sum+"...");
}

HTagBox::~HTagBox()
{
    delete ui;
}
