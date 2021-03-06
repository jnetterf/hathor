#include "hartistbox.h"
#include "ui_hartistbox.h"
#include "hloginwidget.h"
#include "hmainwindow.h"
#include "kfadewidgeteffect.h"

HArtistBox* HArtistBox::getBox(HArtist &rep) {
    return new HArtistBox(rep);
}

HArtistBox::HArtistBox(HArtist &rep, QWidget *parent) :
    HGrowingWidget(parent),
    s_rep(rep),
    ui(new Ui::HArtistBox)
{
    ui->setupUi(this);
    ui->label_artistName->setText("<A href=\"more\">"+rep.getName()+"</A>");
    connect(ui->label_artistName,SIGNAL(linkActivated(QString)),this,SLOT(requestContext()));
    connect(this,SIGNAL(contextRequested(HArtist&)),HMainWindow::singleton(),SLOT(showContext(HArtist&)));
    s_priority[1].push_back(s_rep.sendTagNames(this,"setTagNames"));
    s_priority[1].push_back(s_rep.sendBioShort(this,"setBioShort"));
    setMaximumHeight(130);
    readjustPriorities();
}

HArtistBox::~HArtistBox()
{
    delete ui;
}

void HArtistBox::showEvent(QShowEvent *e) {
    s_showTime=QTime::currentTime();
//    Q_ASSERT(!ui->label_pic->pixmap());
    s_priority[0].push_back(s_rep.sendPic(HArtist::Large,this,"setPic"));
    QWidget::showEvent(e);
    readjustPriorities();
}
void HArtistBox::hideEvent(QHideEvent *e) {
    ui->label_pic->setPixmap(0);
    QWidget::hideEvent(e);
    readjustPriorities();
}

void HArtistBox::setPic(QImage& p) {
    if(!isVisible()) return;
    if(p.width()!=174) p=p.scaledToWidth(174,Qt::SmoothTransformation);

    ui->label_pic->setPixmap(p);
}

void HArtistBox::setTagNames(QStringList tsl) {
    KFadeWidgetEffect* kwe=0;
    if(s_showTime.msecsTo(QTime::currentTime())>110) kwe=new KFadeWidgetEffect(ui->label_tags);
    for(int i=4;i<tsl.size();i++) {
        tsl.removeAt(i);
    }
    QString tags=tsl.join(", ");
    ui->label_tags->setText(tags);
    if(kwe) kwe->start();
}

void HArtistBox::setBioShort(QString sh) {
    KFadeWidgetEffect* kwe=0;
    if(s_showTime.msecsTo(QTime::currentTime())>110) kwe=new KFadeWidgetEffect(ui->label_desc);
    bool ok=sh.size()>360;
    sh.truncate(360);
    if(ok) sh+="...";
    ui->label_desc->setText(sh);
    if(kwe) kwe->start();
}
