#include "htagbox.h"
#include "ui_htagbox.h"

QHash<QString, HTagBox*> HTagBox::s_map;

HTagBox* HTagBox::getBox(HTag &rep) {
    QString dumbName=rep.getTagName();
    if(s_map.contains(dumbName)) return s_map[dumbName];
    s_map[dumbName] = new HTagBox(rep);
    return s_map[dumbName];
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
