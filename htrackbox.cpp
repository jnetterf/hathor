#include "htrackbox.h"
#include "ui_htrackbox.h"
#include "hmainwindow.h"

HTrackBox::HTrackBox(HTrack &ref, QWidget *parent) :
    HGrowingWidget(parent),
    s_ref(ref),
    ui(new Ui::HTrackBox)
{
    ui->setupUi(this);

    connect(ui->label_trackName,SIGNAL(linkActivated(QString)),this,SLOT(requestContext()));
    connect(this,SIGNAL(contextRequested(HTrack&)),HMainWindow::singleton(),SLOT(showContext(HTrack&)));

    ui->label_plays->setText("<B>"+QString::number(ref.getPlayCount())+"</B> plays/<B>"+QString::number(ref.getUserPlayCount())+"</B> by you");
    ui->label_trackName->setText("<B><A href=\"more\">"+s_ref.getTrackName()+(s_ref.getLoved()?" &#x2665; ":" ")+"by "+s_ref.getArtistName()+"</B></A>");

    QStringList tsl=s_ref.getTagNames();
    for(int i=4;i<tsl.size();i++) {
        tsl.removeAt(i);
    }
    QString tags=tsl.join(", ");
    ui->label_tags->setText(tags);
}

HTrackBox::~HTrackBox()
{
    delete ui;
}
