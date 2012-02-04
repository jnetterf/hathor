#include "hlocalintro.h"
#include "hlocalprovider.h"
#include "ui_hlocalintro.h"

HLocalIntro::HLocalIntro(bool enableSelection, HLocalProvider *parent) :
    QWidget(),
    s_enableSelection(enableSelection),
    s_fsm(new HFileSystemModel),
    s_prov(parent),
    s_settings("Nettek","Local Plugin for Hathor"),
    s_theGreatHash(parent->s_theGreatHash),
    s_theInverseHash(parent->s_theInverseHash),
    ui(new Ui::HLocalIntro)
{
    ui->setupUi(this);
//    show();

    if(enableSelection) {
        ui->progressBar_scan->hide();
        ui->label_scan->hide();
        s_fsm->setRootPath(QDir::rootPath());
        s_fsm->setFilter(QDir::AllDirs|QDir::NoDotAndDotDot);
        ui->treeView_fs->setModel(s_fsm);
        ui->treeView_fs->hideColumn(1);
        ui->treeView_fs->hideColumn(2);
        ui->treeView_fs->hideColumn(3);
        ui->treeView_fs->hideColumn(4);
        ui->treeView_fs->header()->hide();
        connect(ui->pushButton,SIGNAL(pressed()),this,SLOT(go()));
    }
}

void HLocalIntro::disableSelection() {
    ui->progressBar_scan->show();
    ui->label_scan->show();
    delete ui->pushButton;
    delete ui->treeView_fs;
    delete ui->label_quest;
    delete ui->label;
    ui->gridLayout_2->removeItem(ui->horizontalSpacer_3);
}

void HLocalIntro::go() {
    disableSelection();

    QStringList dirs;
    QStringList dirsNot;
    if(!s_enableSelection) {
        dirs=s_settings.value("Directories").toStringList();
        dirsNot=s_settings.value("DirectoriesNot").toStringList();
    } else {
        for(int i=0;i<s_fsm->s_checked.size();i++) {
            (s_fsm->s_checked.values()[i]?dirs:dirsNot).push_back(s_fsm->s_checked.keys()[i]);  // I <3 this line with a passion.
        }
    }

    if(!s_settings.value("The Great Hash").isNull()) s_theGreatHash=s_settings.value("The Great Hash").toHash();
    if(!s_settings.value("The Inverse Hash").isNull()) s_theInverseHash=s_settings.value("The Inverse Hash").toHash();

    // rescan (FIX THIS)

    QStringList files;

    for(int i=0;i<dirs.size();i++) {
        QDirIterator it(dirs[i], QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        while(it.hasNext()) {
            it.next();

            bool ok=1;
            for(int j=0;j<dirsNot.size();j++) {
                if(it.fileInfo().filePath().startsWith(dirsNot[j])&&dirs[i].size()<dirsNot[j].size()) { ok=0; break; }
            }
            if(!ok) continue;

            if(it.fileInfo().completeSuffix().toLower() == "mp3"||it.fileInfo().completeSuffix().toLower() == "ogg"||it.fileInfo().completeSuffix().toLower() == "wav"||
                    it.fileInfo().completeSuffix().toLower() == "flac") files.push_back(it.filePath());
        }
    }

    Phonon::MediaObject mo;
    ui->progressBar_scan->setRange(0,files.size());
    for(int i=0;i<files.size();i++) {
        ui->progressBar_scan->setValue(i);
        if(s_theInverseHash.contains(files[i])) continue;
        Phonon::MediaSource ms(files[i]);
        mo.setCurrentSource(ms);

        //
        // BEHOLD - THE GREATEST HACK MANKIND HAS EVER WITNESSED: (no really, fix this)
        //

        mo.play();
        mo.pause();
        QEventLoop loop;
        connect(&mo,SIGNAL(metaDataChanged()),&loop,SLOT(quit()));
        QTimer::singleShot(20,&loop,SLOT(quit()));
        loop.exec();
        if(!mo.metaData("ARTIST").size()||!mo.metaData("ARTIST").first().size()||!mo.metaData("TITLE").size()||!mo.metaData("TITLE").first().size()) {
            connect(&mo,SIGNAL(metaDataChanged()),&loop,SLOT(quit()));
            QTimer::singleShot(20,&loop,SLOT(quit()));
            loop.exec();
        }

        if(mo.metaData("ARTIST").size()&&mo.metaData("TITLE").size()) {
            s_theGreatHash.insert(s_prov->local_standardized(mo.metaData("ARTIST").first()+"__"+mo.metaData("TITLE").first()),files[i]);
            s_theInverseHash.insert(files[i],s_prov->local_standardized(mo.metaData("ARTIST").first()+"__"+mo.metaData("TITLE").first()));
        }

        mo.stop();
        //
        // End.
        //
    }
    s_settings.setValue("The Great Hash",QVariant::fromValue(s_theGreatHash));
    s_settings.setValue("The Inverse Hash",QVariant::fromValue(s_theInverseHash));

    s_settings.setValue("Directories",dirs);
    s_settings.setValue("DirectoriesNot",dirsNot);
    delete this;
}

HLocalIntro::~HLocalIntro()
{
    delete ui;
}
