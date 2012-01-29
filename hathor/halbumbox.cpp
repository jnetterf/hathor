#include "halbumbox.h"
#include "ui_halbumbox.h"
#include "hloginwidget.h"
#include "hmainwindow.h"
#include "kfadewidgeteffect.h"

HAlbumBox::HAlbumBox(HAlbum &album, QWidget *parent) :
    HGrowingWidget(parent),
    s_album(album),
    s_cachedPlayCount(-1),s_cachedListenerCount(-1),s_cachedUserPlayCount(-1),
    ui(new Ui::HAlbumBox)
{
    ui->setupUi(this);

    connect(ui->label_title,SIGNAL(linkActivated(QString)),this,SLOT(requestContext()));
    connect(this,SIGNAL(contextRequested(HAlbum&)),HMainWindow::singleton(),SLOT(showContext(HAlbum&)));

    album.sendPic(HAlbum::Large,this,"setPixmap");
    album.sendTagNames(this,"setTagNames");
    ui->label_title->setText("<B><A href=\"more\">"+album.getAlbumName()+"</A></B>");
    ui->label_artist->setText("by <B>"+album.getArtistName()+"</B>");
    album.sendPlayCount(this,"setPlayCount");
    album.sendListenerCount(this,"setListenerCount");
    album.sendUserPlayCount(this,"setUserPlayCount");
}

HAlbumBox::~HAlbumBox()
{
    delete ui;
}


void HAlbumBox::setPlayCount(int count) {
    s_cachedPlayCount=count;
    updateCounts();
}

void HAlbumBox::setListenerCount(int count) {
    s_cachedListenerCount=count;
    updateCounts();
}

void HAlbumBox::setUserPlayCount(int count) {
    s_cachedUserPlayCount=count;
    updateCounts();
}

void HAlbumBox::updateCounts() {
    ui->label_summary->setText("<B>"+QString::number(s_cachedPlayCount)+"</B> plays by "+QString::number(s_cachedListenerCount)+" listeners<br><B>"+QString::number(s_cachedUserPlayCount)+"</B> plays in your library");
}


void HAlbumBox::setPixmap(QPixmap p) {
    KFadeWidgetEffect* kwe=new KFadeWidgetEffect(this);
    ui->label_icon->setPixmap(p.scaledToWidth(174,Qt::SmoothTransformation));
    ui->label_icon->adjustSize();
    adjustSize();
    kwe->start();
}

void HAlbumBox::setTagNames(QStringList tsl) {
    for(int i=4;i<tsl.size();i++) {
        tsl.removeAt(i);
    }
    QString tags=tsl.join(", ");
    ui->label_tags->setText(tags);
    ui->label_tags->adjustSize();
}
