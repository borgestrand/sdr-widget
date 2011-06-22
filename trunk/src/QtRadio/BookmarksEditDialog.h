#ifndef BOOKMARKSEDITDIALOG_H
#define BOOKMARKSEDITDIALOG_H

#include <QDialog>
#include <QVector>
#include <QDebug>

#include "Bookmark.h"
#include "Bookmarks.h"

namespace Ui {
    class BookmarksEditDialog;
}

class BookmarksEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookmarksEditDialog(QWidget *parent,Bookmarks* bookmarks);
    ~BookmarksEditDialog();

    void setTitle(QString t);
    void setBand(QString b);
    void setFrequency(QString f);
    void setMode(QString m);
    void setFilter(QString f);

public slots:
    void deleteBookmark();
    void bookmarkRowChanged(int);
    void updateBookmark();

signals:
    void bookmarkDeleted(int entry);
    void bookmarkSelected(int entry);
    void bookmarkUpdated(int entry,QString title);

private:
    Ui::BookmarksEditDialog *ui;
    QVector<Bookmark*> bookmarks;
};

#endif // BOOKMARKSEDITDIALOG_H
