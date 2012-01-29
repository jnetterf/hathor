#include "hartistbox.h"
#include "ui_hartistbox.h"
#include "hloginwidget.h"
#include "hmainwindow.h"
#include "kfadewidgeteffect.h"

HArtistBox::HArtistBox(HArtist &rep, QWidget *parent) :
    HGrowingWidget(parent),
    s_rep(rep),
    ui(new Ui::HArtistBox)
{
    ui->setupUi(this);
    ui->label_artistName->setText("<A href=\"more\">"+rep.getName()+"</A>");
    connect(ui->label_artistName,SIGNAL(linkActivated(QString)),this,SLOT(requestContext()));
    connect(this,SIGNAL(contextRequested(HArtist&)),HMainWindow::singleton(),SLOT(showContext(HArtist&)));
    rep.sendPic(HArtist::Mega,this,"setPic");
    s_rep.sendTagNames(this,"setTagNames");

    s_rep.sendBioShort(this,"setBioShort");
}

HArtistBox::~HArtistBox()
{
    delete ui;
}

void HArtistBox::setPic(QPixmap p) {
    KFadeWidgetEffect* kwe=new KFadeWidgetEffect(this);
    ui->label_pic->setPixmap(p.scaledToWidth(174,Qt::SmoothTransformation));
    kwe->start();
}

void HArtistBox::setTagNames(QStringList tsl) {
    for(int i=4;i<tsl.size();i++) {
        tsl.removeAt(i);
    }
    QString tags=tsl.join(", ");
    ui->label_tags->setText(tags);

}

void HArtistBox::setBioShort(QString sh) {
    bool ok=sh.size()>360;
    sh.truncate(360);
    if(ok) sh+="...";
    ui->label_desc->setText(sh);
}
