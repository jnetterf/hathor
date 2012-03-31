#ifndef HCONFIGCONTEXT_H
#define HCONFIGCONTEXT_H

#include <QWidget>
#include "huser.h"
#include <lastfm/ws.h>
#include "hbrowser.h"
#include "hnettloger.h"

namespace Ui {
class HConfigContext;
}

class HConfigContext : public QWidget
{
    Q_OBJECT
    static HConfigContext* s_singleton;
    
    explicit HConfigContext(QWidget *parent = 0);
    HBrowser s_browser;
public:
    static HConfigContext* singleton() { if(!s_singleton) s_singleton=new HConfigContext; return s_singleton; }
    ~HConfigContext();
    void showEvent(QShowEvent *) {
        **HUser::get(lastfm::ws::Username).sendPic(HUser::Medium,this,"setMePic")=1;
    }

public slots:
    void getShouts();
    void setMePic(QImage &p);
    void validateShout();
    void shout();
    
private:
    Ui::HConfigContext *ui;
};

#endif // HCONFIGCONTEXT_H
