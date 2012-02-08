#include "hmaincontextoptions.h"
#include "ui_hmaincontextoptions.h"

#include <QButtonGroup>
#include <QIcon>

HMainContextOptions::HMainContextOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HMainContextOptions)
{
    ui->setupUi(this);
    QButtonGroup*b1=new QButtonGroup(this);
    b1->addButton(ui->toolButton_album);
    b1->addButton(ui->toolButton_list);
    b1->addButton(ui->toolButton_play);

    QButtonGroup*b2=new QButtonGroup(this);
    b2->addButton(ui->toolButton_allArtists);
    b2->addButton(ui->toolButton_suggestions);
    b2->addButton(ui->toolButton_local);

    connect(ui->toolButton_album,SIGNAL(toggled(bool)),this,SLOT(updateIcons()));
    connect(ui->toolButton_list,SIGNAL(toggled(bool)),this,SLOT(updateIcons()));
    connect(ui->toolButton_play,SIGNAL(toggled(bool)),this,SLOT(updateIcons()));
    updateIcons();

    connect(ui->toolButton_allArtists,SIGNAL(toggled(bool)),this,SLOT(onAllArtistsToggled(bool)));
    connect(ui->toolButton_suggestions,SIGNAL(toggled(bool)),this,SLOT(onSuggestionsToggled(bool)));
    connect(ui->toolButton_local,SIGNAL(toggled(bool)),this,SLOT(onLocalToggled(bool)));

    connect(ui->toolButton_album,SIGNAL(toggled(bool)),this,SLOT(onAlbumModeToggled(bool)));
    connect(ui->toolButton_list,SIGNAL(toggled(bool)),this,SLOT(onListModeToggled(bool)));
    connect(ui->toolButton_play,SIGNAL(toggled(bool)),this,SLOT(onPlayModeToggled(bool)));
}

void HMainContextOptions::updateIcons() {
    if(ui->toolButton_album->isChecked()) {
        ui->toolButton_album->setIcon(QIcon(":/icons/view_album_sel.png"));
    } else {
        ui->toolButton_album->setIcon(QIcon(":/icons/view_album.png"));
    }

    if(ui->toolButton_list->isChecked()) {
        ui->toolButton_list->setIcon(QIcon(":/icons/view_list_sel.png"));
    } else {
        ui->toolButton_list->setIcon(QIcon(":/icons/view_list.png"));
    }

    if(ui->toolButton_play->isChecked()) {
        ui->toolButton_play->setIcon(QIcon(":/icons/media-play-alt-sel.png"));
    } else {
        ui->toolButton_play->setIcon(QIcon(":/icons/media-play-alt.png"));
    }

}

HMainContextOptions::~HMainContextOptions()
{
    delete ui;
}
