#ifndef HUSER_H
#define HUSER_H

#include <QStringList>
#include <QPixmap>

class HArtist;

class HUser
{
    QString s_username;
public:
    static HUser& get(QString username);

    enum PictureSize {
        Small=0,
        Medium=1,
        Large=2,
        Mega=3
    };

    QString getUsername() { return s_username; }

    QString getRealName();
    QPixmap getPic(PictureSize pic);
    int getAge();
    bool getMale();
    int getPlayCount();
    int getPlaylists();

private:
    static QMap<QString, HUser*> _map;
    HUser(QString username);  // use HUser::get(name)

    struct InfoData {
        QString realName;
        QString pics[4];
        QString country;
        int age;
        bool male;
        int playCount;
        int playlists;
        bool got;
        InfoData() : got(0) {}

        void getData(QString user);
    } s_infoData;

    struct PictureData {
        bool got[4];
        QPixmap pics[4];
        PictureData() { for(int i=0;i<4;i++) got[i]=0; }
        void getData(QString url,PictureSize size);
    } s_pictureData;

private:
    //Degenerate copy and assignment
    HUser(const HUser&);
    HUser& operator=(const HUser&);
};

#endif // HUSER_H
