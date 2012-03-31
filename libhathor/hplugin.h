#ifndef HPLUGIN_H
#define HPLUGIN_H

#include "libhathor_global.h"
#include "habstractmusicinterface.h"

class LIBHATHORSHARED_EXPORT HPlugin {
public:
    /*! If your plugin should support playing music, implement HAbstractTrackProvider, and return a singleton to it in this function. Else it should return 0 */
    virtual HAbstractTrackProvider* trackProvider() = 0;

    /*! When the plugin is initialized, initWidget() is shown until it is destroyed. Returning 0 causes no input widget to be shown. Use this for login input, or if your plugin takes a while to start. */
    virtual QWidget* initWidget() = 0;

    /*! Examples of names are 'Rdio', 'last.fm' or 'local' */
    virtual QString name() = 0;

    /*! Return a QSet<QString> of commands this plugin supports.
     *
     * Commands should be in the send pattern, and should be accessed in the 'send' method. See HTrack for more information.
     *
     * Commands sent from aritsts should have the previx 'HArtist::' For example, if this implements sendTagNames have 'HArtist::sendTagNames' in the return.
     * Commands sent from albums should have the previx 'HAlbum::'
     * Commands sent from tracks should have the previx 'HTrack::'
     * Commands sent from users should have the previx 'HUser::'
     * Commands sent from tags should have the previx 'HTag::'
     */
    virtual QSet<QString> abilities() = 0;



    /*! Do a command, returning a reference to a priority.
     *
     * This follows the send pattern. For more information, see HTrack.
     *
     * A good backend idea might be to implement a QHash (or similar) between QStrings and true c++ functors.
     */
    virtual int** send(const QString& command,HObject* mobj,QObject* object,QString slot,QObject* guest)=0;

    /*! Return how good this plugin is at an ability.
     *
     * If the ability is not implemented, return -1.
     */
    virtual int getScore(const QString& command,HObject* mobj=0,QObject* object=0,QString slot="",QObject* guest=0)=0;

    /*! Return wheter this command will use only local resources.
     */
    virtual bool isLocal(const QString& command,HObject* mobj=0,QObject* object=0,QString slot="",QObject* guest=0)=0;

    /*! Returns a small widget (width 395!!!) which configures the plugin. This will be visible in the configuration context.
     *
     * This function MUST return an actual widget, which should be a singleton! It MUST give a brief description of what the plugin
     * does and it MUST have an option to remove all settings and cache.
     */
    virtual QWidget* configurationWidget()=0;
};

class LIBHATHORSHARED_EXPORT HPluginManager {
    QList<HPlugin*> s_pll;
    static HPluginManager* s_singleton;
public:
    void regPlugin(HPlugin* a) {s_pll.push_back(a);}

    int** send(const QString& command,HObject* mobj,QObject* object,QString slot,QObject* guest=0) {
        int ts=0, ti=-1;
        for(int i=0;i<s_pll.size();i++) {
            if(s_pll[i]->getScore(command)>ts) {
                ti=i;
                ts=s_pll[i]->getScore(command);
            }
        }
        if(ti==-1) {
            return 0;
        } else {
            return s_pll[ti]->send(command,mobj,object,slot,guest);
        }
    }

    int** sendLocal(const QString& command,HObject* mobj,QObject* object,QString slot,QObject* guest=0) {
        int ts=0, ti=-1;
        for(int i=0;i<s_pll.size();i++) {
            if(s_pll[i]->isLocal(command)&&s_pll[i]->getScore(command)>ts) {
                ti=i;
                ts=s_pll[i]->getScore(command);
            }
        }
        if(ti==-1) {
            return 0;
        } else {
            return s_pll[ti]->send(command,mobj,object,slot,guest);
        }
    }
    static HPluginManager* singleton() { return (s_singleton=(s_singleton?s_singleton:new HPluginManager)); }
};

Q_DECLARE_INTERFACE(HPlugin, "com.nettek.hathor.plugin/0.01")

#endif // HPLUGIN_H
