#ifndef BOOKMARKSDIALOG_H
#define BOOKMARKSDIALOG_H

#include <QDialog>
#include <QVector>

#include "Bookmark.h"
#include "Bookmarks.h"

namespace Ui {
    class BookmarksDialog;
}

class BookmarksDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookmarksDialog(QWidget *parent,Bookmarks* bookmarks);
    ~BookmarksDialog();

    int getSelected();

private:
    Ui::BookmarksDialog *ui;
};

#endif // BOOKMARKSDIALOG_H
