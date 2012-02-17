#include "htagbox.h"
#include "ui_htagbox.h"

HTagBox* HTagBox::getBox(HTag &rep) {
    return new HTagBox(rep);
}

HTagBox::HTagBox(HTag &rep, QWidget *parent) :
    HGrowingWidget(parent),
    s_rep(rep),
    ui(new Ui::HTagBox)
{
    ui->setupUi(this);
    ui->label_tag->setText("<A href=\"more\">"+rep.getTagName()+"</A>");
    rep.sendSummary(this,"setSummary");
}

void HTagBox::setSummary(QString sum) {
    sum.truncate(120);
    ui->label_desc->setText(sum+"...");
}

HTagBox::~HTagBox()
{
    delete ui;
}
