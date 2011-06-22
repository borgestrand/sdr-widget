#ifndef XVTR_H
#define XVTR_H

#include <QObject>
#include <QAction>
#include <QString>
#include <QSettings>

#include "XvtrEntry.h"

class Xvtr : public QObject {
    Q_OBJECT

public:
    Xvtr();
    void buildMenu(QMenu* menu);
    void add(QString,long long,long long,long long,long long,int,int);
    void del(int index);
    void select(QAction* action);
    QString getTitle();
    long long getMinFrequency();
    long long getMaxFrequency();
    long long getIFFrequency();

    long long getFrequency();
    int getMode();
    int getFilter();
    void setFrequency(long long f);
    void setMode(int m);
    void setFilter(int f);

    int count();
    XvtrEntry* getXvtrAt(int index);

    void loadSettings(QSettings* settings);
    void saveSettings(QSettings* settings);

public slots:
    void triggered();

signals:
    void xvtrSelected(QAction*);

private:
    QVector<XvtrEntry*> xvtrs;     // list of XVTR entries
    QAction *currentXvtrAction;    // action for current entry
    XvtrEntry* currentXvtr;        // currently selected
};

#endif // XVTR_H
