#include <QDebug>

#include <QAction>
#include <QMenu>

#include "Xvtr.h"
#include "Mode.h"

Xvtr::Xvtr() {
    currentXvtr=NULL;
}

// SLOTS
void Xvtr::triggered() {
    QAction *action = qobject_cast<QAction *>(sender());
    emit xvtrSelected(action);
}

// public functions

void Xvtr::loadSettings(QSettings* settings) {
    int count;
    QString s;
    QString title;
    long long minFrequency;
    long long maxFrequency;
    long long ifFrequency;
    long long frequency;
    int mode;
    int filter;

    settings->beginGroup("XVTR");
    if(settings->contains("count")) {
        count=settings->value("count").toInt();
        for(int i=0;i<count;i++) {
            s.sprintf("title.%d",i);
            title=settings->value(s).toString();
            s.sprintf("minFrequency.%d",i);
            minFrequency=settings->value(s).toLongLong();
            s.sprintf("maxFrequency.%d",i);
            maxFrequency=settings->value(s).toLongLong();
            s.sprintf("ifFrequency.%d",i);
            ifFrequency=settings->value(s).toLongLong();
            s.sprintf("frequency.%d",i);
            if(settings->contains(s)) {
                frequency=settings->value(s).toLongLong();
            } else {
                frequency=minFrequency;
            }
            s.sprintf("mode.%d",i);
            if(settings->contains(s)) {
                mode=settings->value(s).toLongLong();
            } else {
                mode=MODE_USB;
            }
            s.sprintf("filter.%d",i);
            if(settings->contains(s)) {
                filter=settings->value(s).toLongLong();
            } else {
                filter=5;
            }



            add(title,minFrequency,maxFrequency,ifFrequency,frequency,mode,filter);
        }
    }
    settings->endGroup();
}

void Xvtr::saveSettings(QSettings* settings) {
    QString s;
    XvtrEntry* entry;

    settings->beginGroup("XVTR");
    settings->setValue("count",xvtrs.count());
    for(int i=0;i<xvtrs.count();i++) {
        entry=xvtrs.at(i);
        s.sprintf("title.%d",i);
        settings->setValue(s,entry->getTitle());
        s.sprintf("minFrequency.%d",i);
        settings->setValue(s,entry->getMinFrequency());
        s.sprintf("maxFrequency.%d",i);
        settings->setValue(s,entry->getMaxFrequency());
        s.sprintf("ifFrequency.%d",i);
        settings->setValue(s,entry->getIFFrequency());
    }
    settings->endGroup();
}

void Xvtr::buildMenu(QMenu* menu) {
    XvtrEntry* xvtr;
    QAction* action;
    menu->clear();
    for(int i=0;i<xvtrs.size();++i) {
        xvtr=xvtrs.at(i);
        action=new QAction(xvtr->getTitle(),this);
        menu->addAction(action);

        //need an event for the new action
        connect(action,SIGNAL(triggered()),this,SLOT(triggered()));
    }
}

void Xvtr::add(QString title, long long minFrequency, long long maxFrequency, long long ifFrequency,long long frequency,int mode,int filter) {
    qDebug()<<"Xvtr::add"<<title;
    xvtrs.append(new XvtrEntry(title,minFrequency,maxFrequency,ifFrequency,frequency,mode,filter));
}

void Xvtr::del(int index) {
    xvtrs.remove(index);
}

void Xvtr::select(QAction* action) {
    if(currentXvtrAction!=NULL) {
        currentXvtrAction->setChecked(false);
    }
    // find the entry
    for(int i=0;i<xvtrs.size();i++) {
        currentXvtr=xvtrs.at(i);
        if(action->text()==currentXvtr->getTitle()) {
            break;
        } else {
            currentXvtr=NULL;
        }
    }

    if(currentXvtr!=NULL) {
        action->setChecked(true);
        currentXvtrAction=action;
    }

}

QString Xvtr::getTitle() {
    return currentXvtr->getTitle();
}

long long Xvtr::getMinFrequency() {
    return currentXvtr->getMinFrequency();
}

long long Xvtr::getMaxFrequency() {
    return currentXvtr->getMaxFrequency();
}

long long Xvtr::getIFFrequency() {
    return currentXvtr->getMaxFrequency();
}


int Xvtr::count() {
    return xvtrs.count();
}

XvtrEntry* Xvtr::getXvtrAt(int index) {
    return xvtrs.at(index);
}

long long Xvtr::getFrequency() {
    return currentXvtr->getFrequency();
}

int Xvtr::getMode() {
    return currentXvtr->getMode();
}

int Xvtr::getFilter() {
    return currentXvtr->getFilter();
}

void Xvtr::setFrequency(long long f) {
    currentXvtr->setFrequency(f);
}

void Xvtr::setMode(int m) {
    currentXvtr->setMode(m);
}

void Xvtr::setFilter(int f) {
    currentXvtr->setFilter(f);
}
