#include "hconfigcontext.h"
#include "ui_hconfigcontext.h"
#include "hnettloger.h"

#include "hshoutbox.h"
#include "lastfm.h"
#include "hlfmwebloginaction.h"
#include <QTimer>

HConfigContext* HConfigContext::s_singleton=0;

HConfigContext::HConfigContext(QWidget *parent) :
    QWidget(parent),
    s_browser(0,false),
    ui(new Ui::HConfigContext)
{
    ui->setupUi(this);
    ui->widget_comments->setLayout(new QVBoxLayout);
    HUser::get(lastfm::ws::Username).sendPic(HUser::Medium,this,"setMePic");
    connect(&s_browser,SIGNAL(ready()),this,SLOT(getShouts()));
    connect(ui->textEdit_shout,SIGNAL(textChanged()),this,SLOT(validateShout()));
    connect(ui->pushButton_post,SIGNAL(clicked()),this,SLOT(shout()));
    s_browser.loadPage("http://www.last.fm/group/Hathor+Users");
    validateShout();

    connect(ui->comboBox_log,SIGNAL(activated(int)),this,SLOT(setLogPolicy()));
    connect(ui->checkBox_scrobble,SIGNAL(clicked(bool)),this,SLOT(setLogPolicy()));
    connect(ui->checkBox_similar,SIGNAL(toggled(bool)),this,SLOT(setLogPolicy()));
}

HConfigContext::~HConfigContext()
{
    delete ui;
}

void HConfigContext::getShouts() {
    disconnect(&s_browser,SIGNAL(ready()),this,SLOT(getShouts()));
    QString l=s_browser.html();
    l.truncate(l.indexOf("<div id=\"justCantGetEnough\" class=\"clearit\">"));
    QList<HShout*> shouts;
    while(1) {
        int n=l.indexOf("</blockquote>");
        if(n==-1) break;
        QString f=l;
        f.truncate(n);
        l.remove(0,n+13);

        QString v=f;
        int m=v.lastIndexOf("http://cdn.last.fm/flatness/clear.gif\"></span>");
        if(m==-1) continue;
        v.remove(0,m+46);
        v.truncate(v.indexOf("</a>"));

        f.remove(0,f.indexOf("<blockquote>")+12);
        f.remove('\n');
        f.remove('\r');
        while(f.startsWith(' ')) f.remove(0,1);
        while(f.endsWith(' ')) f.chop(1);
        shouts.push_back( new HShout(f,HUser::get(v),""));
    }


    int i;
    for(i=0;i<shouts.size()&&i<15;i++) {
        HShoutBox* ab=new HShoutBox(*shouts[i],this);
        ui->widget_comments->layout()->addWidget(ab);

    }
    ui->label_moreShoutbox->hide();
}

void HConfigContext::setMePic(QPixmap& pic) {
    if(pic.width()!=70) pic=pic.scaledToWidth(70,Qt::SmoothTransformation);
    ui->label_you->setPixmap(pic);
}

void HConfigContext::validateShout() {
    ui->pushButton_post->setEnabled(ui->textEdit_shout->toPlainText().size()&&ui->textEdit_shout->toPlainText().size()<1000);
    ui->label_characterUse->setText(QString::number(ui->textEdit_shout->toPlainText().size())+"/1000 characters used");
}

void HConfigContext::shout() {
    HLfmWebManager::singleton()->shout("http://www.last.fm/group/Hathor+Users",ui->textEdit_shout->toPlainText());
    ui->textEdit_shout->setText("");
    ui->label_characterUse->setText("Sent!");
}
