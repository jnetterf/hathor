#ifndef HLOCALINTRO_H
#define HLOCALINTRO_H

#include <QWidget>
#include <QFileSystemModel>
#include <QDebug>
#include <QDir>
#include <QSettings>

namespace Ui {
class HLocalIntro;
}

class HFileSystemModel : public QFileSystemModel {
    Q_OBJECT
    QHash<QString,bool> s_checked;
public:
    friend class HLocalIntro;
    QVariant data(const QModelIndex &index, int role) const {
        if(role==Qt::CheckStateRole) {
            if(s_checked.contains(filePath(index))) {
                return s_checked.value(filePath(index),0)?Qt::Checked:Qt::Unchecked;
            }
            int ans=-1;
            int len=0;
            for(int i=0;i<s_checked.size();i++) {
                if(filePath(index).startsWith(s_checked.keys()[i])&&(s_checked.keys()[i].size()>len)) {
                    len=s_checked.keys()[i].size();
                    ans=s_checked.values()[i];
                }
            }
            if(ans!=-1) return ans?Qt::Checked:Qt::Unchecked;
            else return Qt::Unchecked;
        } else {
            return QFileSystemModel::data(index,role);
        }
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) {
        qDebug()<<"!!!";
        if(role==Qt::CheckStateRole) {
            s_checked[filePath(index)]=((value!=Qt::Unchecked)?1:0);
            emit dataChanged(index,index.sibling(index.row()+1,index.column()));
            return 1;
        } else {
            return QFileSystemModel::setData(index,value,role);
        }
    }

    Qt::ItemFlags flags(const QModelIndex &index) const
     {
         if (!index.isValid())
             return Qt::ItemIsEnabled;

         return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
     }
};

class HLocalProvider;

class HLocalIntro : public QWidget
{
    Q_OBJECT
    
    bool s_enableSelection;
    HFileSystemModel* s_fsm;
    HLocalProvider* s_prov;
    QSettings s_settings;

    QVariantHash& s_theGreatHash;
    QVariantHash& s_theInverseHash;

public:
    explicit HLocalIntro(bool enableSelection,HLocalProvider *parent);
    void disableSelection();
    ~HLocalIntro();
public slots:
    void go();
    
private:
    Ui::HLocalIntro *ui;
};

#endif // HLOCALINTRO_H
