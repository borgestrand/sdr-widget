#ifndef BOOKMARKDIALOG_H
#define BOOKMARKDIALOG_H

#include "ui_Bookmark.h"

class BookmarkDialog : public QDialog {
    Q_OBJECT
public:
    BookmarkDialog();

    void setTitle(QString t);
    void setBand(QString b);
    void setFrequency(QString f);
    void setMode(QString m);
    void setFilter(QString f);

    QString getTitle();

private:
    Ui::BookmarkDialog widget;
};

#endif // BOOKMARKDIALOG_H
