#include "halbumbox.h"
#include "ui_halbumbox.h"
#include "hloginwidget.h"

HAlbumBox::HAlbumBox(HAlbum &album, QWidget *parent) :
    HGrowingWidget(parent),
    s_album(album),
    ui(new Ui::HAlbumBox)
{
    ui->setupUi(this);

    connect(ui->label_title,SIGNAL(linkActivated(QString)),this,SLOT(requestContext()));
    connect(this,SIGNAL(contextRequested(HAlbum&)),HLoginWidget::singleton,SLOT(onShowContext(HAlbum&)));

    ui->label_icon->setPixmap(album.getPic(HAlbum::Large));
    ui->label_title->setText("<B><A href=\"more\">"+album.getAlbumName()+"</A></B>");
    ui->label_artist->setText("by <B>"+album.getArtistName()+"</B>");
    ui->label_summary->setText("<B>"+QString::number(album.getPlayCount())+"</B> plays by "+QString::number(album.getListenerCount())+" listeners<br><B>"+QString::number(album.getUserPlayCount())+"</B> plays in your library");
    QStringList tsl=album.getTagNames();
    for(int i=4;i<tsl.size();i++) {
        tsl.removeAt(i);
    }
    QString tags=tsl.join(", ");
    ui->label_tags->setText(tags);
}

HAlbumBox::~HAlbumBox()
{
    delete ui;
}
