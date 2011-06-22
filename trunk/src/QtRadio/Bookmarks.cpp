#include <QDebug>

#include <QAction>
#include <QMenu>

#include "Bookmarks.h"


Bookmarks::Bookmarks() {
    currentBookmark=NULL;
}

// SLOTS
void Bookmarks::triggered() {
    QAction *action = qobject_cast<QAction *>(sender());
    qDebug()<<"Bookmarks::triggered"<<action->text();
    emit bookmarkSelected(action);
}

void Bookmarks::buildMenu(QMenu* menu) {
    Bookmark* bookmark;
    QAction* action;

    qDebug()<<"Bookmarks::buildMenu"<<bookmarks.size();
    menu->clear();
    for(int i=0;i<bookmarks.size();++i) {
        bookmark=bookmarks.at(i);
        action=new QAction(bookmark->getTitle(),this);
        menu->addAction(action);

        //need an event for the new action
        connect(action,SIGNAL(triggered()),this,SLOT(triggered()));

        qDebug()<<"  added "<<action->text();
    }
}

void Bookmarks::saveSettings(QSettings* settings) {
    QString s;
    Bookmark* bookmark;
    qDebug()<<"Bookmarks::saveSettings";
    settings->beginGroup("Bookmarks");
    settings->setValue("entries",bookmarks.count());
    for(int i=0;i<bookmarks.count();i++) {
        bookmark=bookmarks.at(i);
        s.sprintf("title.%d",i);
        settings->setValue(s,bookmark->getTitle());
        s.sprintf("band.%d",i);
        settings->setValue(s,bookmark->getBand());
        s.sprintf("frequency.%d",i);
        settings->setValue(s,bookmark->getFrequency());
        s.sprintf("mode.%d",i);
        settings->setValue(s,bookmark->getMode());
        s.sprintf("filter.%d",i);
        settings->setValue(s,bookmark->getFilter());
    }
    settings->endGroup();
}

void Bookmarks::loadSettings(QSettings* settings) {
    QString s;
    Bookmark* bookmark;

    qDebug()<<"Bookmarks::loadSettings";
    settings->beginGroup("Bookmarks");
    if(settings->contains("entries")) {
        int entries=settings->value("entries").toInt();
        qDebug()<<" entries="<<entries;
        for(int i=0;i<entries;i++) {
            s.sprintf("title.%d",i);
            if(settings->contains(s)) {
                bookmark=new Bookmark();
                bookmark->setTitle(settings->value(s).toString());
                s.sprintf("band.%d",i);
                bookmark->setBand(settings->value(s).toLongLong());
                s.sprintf("frequency.%d",i);
                bookmark->setFrequency(settings->value(s).toLongLong());
                s.sprintf("mode.%d",i);
                bookmark->setMode(settings->value(s).toInt());
                s.sprintf("filter.%d",i);
                bookmark->setFilter(settings->value(s).toInt());
                bookmarks.append(bookmark);
                qDebug()<<"  append "<<bookmark->getTitle();
            } else {
                qDebug()<<"  no entry for"<<s;
            }

        }
    } else {
        qDebug()<<"  no entries";
    }
    settings->endGroup();
}

void Bookmarks::add(QString title,int band,long long frequency,int mode,int filter) {
    qDebug() << "Bookmarks::add";
    Bookmark* bookmark=new Bookmark();
    bookmark->setTitle(title);
    bookmark->setBand(band);
    bookmark->setFrequency(frequency);
    bookmark->setMode(mode);
    bookmark->setFilter(filter);
    bookmarks.append(bookmark);
}

void Bookmarks::add(Bookmark* bookmark) {
    qDebug()<<"Bookmarks::add";
    bookmarks.append(bookmark);
}

void Bookmarks::remove(int index) {
    qDebug()<<"Bookmarks::remove";
    bookmarks.remove(index);
}

Bookmark* Bookmarks::at(int index) {
    qDebug()<<"Bookmarks::at";
    return bookmarks.at(index);
}

int Bookmarks::count() {
    qDebug()<<"Bookmarks::count";
    return bookmarks.count();
}

void Bookmarks::select(QAction* action) {
    qDebug()<<"Bookmarks::select";

    // find the entry
    for(int i=0;i<bookmarks.size();i++) {
        currentBookmark=bookmarks.at(i);
        if(action->text()==currentBookmark->getTitle()) {
            break;
        } else {
            currentBookmark=NULL;
        }
    }

}

int Bookmarks::getBand() {
    return currentBookmark->getBand();
}

long long Bookmarks::getFrequency() {
    return currentBookmark->getFrequency();
}

int Bookmarks::getMode() {
    return currentBookmark->getMode();
}

int Bookmarks::getFilter() {
    return currentBookmark->getFilter();
}
