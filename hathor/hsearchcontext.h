#ifndef HSEARCHCONTEXT_H
#define HSEARCHCONTEXT_H

#include <QWidget>

namespace Ui {
class HSearchContext;
}

class HSearchContext : public QWidget
{
    Q_OBJECT
    
    static HSearchContext* s_singleton;
    explicit HSearchContext(QWidget *parent = 0);
    QString s_term;

    int s_albumCount;
    int s_artistCount;
    int s_tagCount;
    int s_trackCount;
    int s_userCount;
    int s_venueCount;
    QList<QWidget*> s_results;
public:
    static HSearchContext* singleton() { return s_singleton=(s_singleton?s_singleton:new HSearchContext); } //could change!!! so don't connect to setSearchTerm
    static void detach() { s_singleton=0; }

    ~HSearchContext();

public slots:
    void setSearchTerm(QString a=""); // singleton could change!!! so don't connect to setSearchTerm

    void getMoreAlbums(QString s);
    void getMoreAlbums_2();
    void getMoreArtists(QString s);
    void getMoreArtists_2();
    void getMoreTags(QString s);
    void getMoreTags_2();
    void getMoreTracks(QString s);
    void getMoreTracks_2();
    void getMoreUsers(QString s);
    void getMoreUsers_2();
    void getMoreVenues(QString s);
    void getMoreVenues_2();

signals:
    void searchTermChanged();
private:
    Ui::HSearchContext *ui;
};

#endif // HSEARCHCONTEXT_H
