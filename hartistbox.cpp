#include "hartistbox.h"
#include "ui_hartistbox.h"
#include "hloginwidget.h"

HArtistBox::HArtistBox(HArtist &rep, QWidget *parent) :
    HGrowingWidget(parent),
    s_rep(rep),
    ui(new Ui::HArtistBox)
{
    ui->setupUi(this);
    ui->label_artistName->setText("<A href=\"more\">"+rep.getName()+"</A>");
    connect(ui->label_artistName,SIGNAL(linkActivated(QString)),this,SLOT(requestContext()));
    connect(this,SIGNAL(contextRequested(HArtist&)),HLoginWidget::singleton,SLOT(onShowContext(HArtist&)));
    ui->label_pic->setPixmap(rep.getPic(HArtist::Large));
    QStringList tsl=s_rep.getTagNames();
    for(int i=4;i<tsl.size();i++) {
        tsl.removeAt(i);
    }
    QString tags=tsl.join(", ");
    ui->label_tags->setText(tags);

    QString sh=rep.getBioShort();
    bool ok=sh.size()>180;
    sh.truncate(180);
    if(ok) sh+="...";
    ui->label_desc->setText(sh);
}

HArtistBox::~HArtistBox()
{
    delete ui;
}
