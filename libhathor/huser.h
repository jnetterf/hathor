#ifndef HUSER_H
#define HUSER_H

#include "huser.h"
#include "libhathor_global.h"
#include <QStringList>
#include <QImage>
#include "htrack.h"

class HArtist;

class LIBHATHORSHARED_EXPORT HUser : public HObject
{
    Q_OBJECT
    QString s_username;
    HCachedPixmap* s_cachedPixmap[4];
    QList< QPair<QObject*,QString> > s_picQueue[4];
public:
    static HUser& get(QString username);

    enum PictureSize {
        Small=0,
        Medium=1,
        Large=2,
        Mega=3
    };

    QString getUsername() { return s_username; }

public slots:
    void sendRealName(QObject* o, QString m); /*QString*/
    int** sendPicNames(PictureSize pic,QObject* o, QString m,QObject* g); /*QString*/
    int** sendPic(PictureSize pic,QObject* o, QString m); /*QImage& */

    void sendPic_2_0(QString pic) { sendPic_2((PictureSize)0,pic); }
    void sendPic_2_1(QString pic) { sendPic_2((PictureSize)1,pic); }
    void sendPic_2_2(QString pic) { sendPic_2((PictureSize)2,pic); }
    void sendPic_2_3(QString pic) { sendPic_2((PictureSize)3,pic); }

    void sendAge(QObject* o, QString m); /*int*/
    void sendGender(QObject* o, QString m); /*bool*/
    void sendPlayCount(QObject* o, QString m); /*int*/
    void sendPlaylists(QObject* o, QString m); /*int*/
//    void getTopTracks(); /*multiple HTrack* */

    void sendPic_2(PictureSize p,QString pic); /* for internal use */

    void removeFromQueue(QObject* o) {
        for(int j=0;j<4;j++) {
            for(int i=0;i<s_picQueue[j].size();i++) {
                if(s_picQueue[j][i].first==o) {
                    s_picQueue[j].removeAt(i);
                    --i;
                }
            }
        }
    }

private:
    static QHash<QString, HUser*> _map;
    HUser(QString username);  // use HUser::get(name)

    struct InfoData : HCachedInfo {
        InfoData(QString username);
        bool process(const QString& data);
    } s_infoData;

private:
    //Degenerate copy and assignment
    HUser(const HUser&);
    HUser& operator=(const HUser&);
};

#endif // HUSER_H
