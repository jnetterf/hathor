#ifndef HRDIOINTERFACE_H
#define HRDIOINTERFACE_H

#include <QString>
#include "hbrowser.h"
#include "hauthaction.h"
#include <QObject>

class HArtist;
class HAlbum;
class HTrack;

class HRdioInterface : public QObject
{
public slots: // Playback and queue:
    void play(bool a = true);
    void pause();
    void next();
    void superNext();
    void prev();
    void seek(int sec);
    void setShuffle(bool);
    void queue(HArtist& artist);
    void queue(HAlbum& album);
    void queue(HTrack& track);

    void play(HArtist& artist);
    void play(HAlbum& album);
    void play(HTrack& track);

    void setVol(double);    //0<=vol<=1
    void setMute(bool);
    void goToPosInQueue(int);
    void clearQueue();

private:
    struct CurrentTrackInfo {
        int duration;
        QString arist,album,name;
        bool clean, expl;
        bool valid;
        CurrentTrackInfo() : valid(0) {}
    } s_currentInfo;

    double s_curPos;

    enum State {
        Paused=0,
        Playing=1,
        Stopped=2,
        Buffering=3,
        PausedB=4   //brought to you by the department of redundency department
    } s_state;

private:
    Q_OBJECT
    QString s_username, s_password;
    QByteArray s_rdioToken,s_rdioSecret, s_oauthToken,s_oauthSecret;
    QString s_playbackKey;

    HBrowser s_browser;
    HAuthAction* s_auth;

    bool s_ready;
    static HRdioInterface* _singleton;

    HRdioInterface(QString username, QString password);
    HRdioInterface(QString rdioToken, QString rdioSecret, QString oauthToken, QString oauthSecret);
public:
    static HRdioInterface* login(QString username, QString password);
    static HRdioInterface* restore();
    static HRdioInterface* singleton() { return _singleton; } //can return null
    bool ok();
    bool ready() { return s_ready; }

    QString search(QString search,QString types);   // gets best token

    QString trackName() { return s_currentInfo.name; }
    QString artistName() { return s_currentInfo.arist; }

public slots:
    void oauth(QByteArray rdioToken, QByteArray rdioSecret, QByteArray oauthToken,QByteArray oauthSecret);
    void setupPlayback();
    void tryPlay();
    void jsCallback(QString cb);
    void keshaTest();

signals:
    void playStateChanged(State newState);
    void positionChanged(double newPos);
    void playingTrackChanged(HTrack& track);
};

#endif // HRDIOINTERFACE_H
