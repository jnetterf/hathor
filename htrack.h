#ifndef HTRACK_H
#define HTRACK_H

#include <QStringList>
#include <QPixmap>
#include <QVariant>

class HArtist;
class HAlbum;
class HTag;
class HShout;

class HTrack
{
    QString s_artist;
    QString s_track;
public:
    static HTrack& get(QString artist, QString trackName);

    enum PictureSize {
        Small=0,
        Medium=1,
        Large=2,
        Mega=3
    };

    QString getArtistName() { return s_artist; }
    QString getTrackName() { return s_track; }
    HArtist& getArtist();

    QStringList getTagNames();
    QStringList getMoreTagNames();
    QList<HTag*> getTags();
    QList<HTag*> getMoreTags();

    QList<HShout*> getShouts();

    int getListenerCount();
    int getPlayCount();
    int getUserPlayCount();
    QString getSummary();
    QString getContent();
    bool getLoved();

    QStringList getAlbumNames();
    QStringList getAlbumArtistNames();
    QList<HAlbum*> getAlbums();

    QList<HTrack*> getSimilar();
    QList<double> getSimilarScores();

private:
    static QMap<QString, HTrack*> _map;
    HTrack(QString artist, QString track);  // use HTrack::get(name)

    struct InfoData {
        QStringList tags;
        QString summary;
        QString content;
        QStringList albumArtists;
        QStringList albumAlbums;
        int listenerCount;
        int playCount;
        int userPlayCount;
        bool loved;
        bool got;
        InfoData() : got(0) {}

        void getData(QString artist,QString track);
    } s_infoData;

    struct ExtraTagData {
        QStringList tags;
        bool got;
        ExtraTagData() : got(0) {}

        void getData(QString artist,QString track);
    } s_extraTagData;

    struct SimilarData {
        QStringList similar;
        QStringList artists;
        QList<QVariant> score;  //double
        bool got;
        SimilarData() : got(0) {}
        void getData(QString artist, QString track);
    } s_similarData;

    struct ShoutData {
        QList<HShout*> shouts;
        bool got;
        ShoutData() : got(0) {}
        void getData(QString artist, QString track);
    } s_shoutData;

private:
    //Degenerate copy and assignment
    HTrack(const HTrack&);
    HTrack& operator=(const HTrack&);
};

#endif // HTRACK_H
