#ifndef HARTIST_H
#define HARTIST_H

#include <QStringList>
#include <QPixmap>
#include <QVariant>
#include <QObject>
#include <QTimer>
#include "halbum.h"
#include "htrack.h"
#include "hshout.h"

struct HTag;

class HArtist : public QObject
{
    Q_OBJECT
public:
    static HArtist& get(QString name);

    enum PictureSize {
        Small=0,
        Medium=1,
        Large=2,
        Mega=3
    };

    QString getName() { return s_name; }
    QPixmap getPic(PictureSize p);
    QStringList getTagNames();
    QList<HTag*> getTags();
    QStringList getMoreTagNames();
    QList<HTag*> getMoreTags();
    int getListenerCount();
    int getPlayCount();
    int getUserPlayCount();
    QString getBio();
    QString getBioShort();
    QList<HAlbum*> getAlbums();
    QList<HTrack*> getTracks();
    QList<HArtist*> getSimilar();
    QList<HShout*> getShouts();
    QList<double> getSimilarScores();

private:
    static QMap<QString, HArtist*> _map;
    HArtist(QString name);  // use HArtist::get(name)

    QString s_name;

    struct PictureData {
        QPixmap pics[4];
        bool got[4];
        PictureData() {for(int i=0;i<4;i++) got[i]=0;}

        void getData(QString url,PictureSize size);
    } s_pictureData;

    struct InfoData {
        QStringList tags;
        QString pics[4];
        QString bio;
        QString bioShort;
        int listenerCount;
        int playCount;
        int userPlayCount;
        bool got;
        InfoData() : got(0) {}

        void getData(QString artist);
    } s_infoData;

    struct ExtraTagData {
        QStringList tags;
        bool got;
        ExtraTagData() : got(0) {}

        void getData(QString artist);
    } s_extraTagData;

    struct AlbumData {
        QStringList albums;
        bool got;
        AlbumData() : got(0) {}
        void getData(QString artist);
    } s_albumData;

    struct TrackData {
        QStringList tracks;
        bool got;
        TrackData() : got(0) {}
        void getData(QString artist);
    } s_trackData;

    struct SimilarData {
        QStringList similar;
        QList<QVariant> score;  //double
        bool got;
        SimilarData() : got(0) {}
        void getData(QString artist);
    } s_similarData;

    struct ShoutData {
        QList<HShout*> shouts;
        bool got;
        ShoutData() : got(0) {}
        void getData(QString artist);
    } s_shoutData;

signals:
    void gotInfoData();

public slots:
    void cache() {
        QTimer::singleShot(0,this,SLOT(getSimpleData()));
    }
    void getSimpleData() {
        s_infoData.getData(getName());
        emit gotInfoData();
    }

private:
    //Degenerate copy and assignment
    HArtist(const HArtist&);
    HArtist& operator=(const HArtist&);
};

#endif // HARTIST_H
