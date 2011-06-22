#include "BookmarksDialog.h"
#include "ui_BookmarksDialog.h"

BookmarksDialog::BookmarksDialog(QWidget *parent,Bookmarks* bookmarks) :
    QDialog(parent),
    ui(new Ui::BookmarksDialog)
{
    ui->setupUi(this);

    // load the bookmarks
    Bookmark* bookmark;
    for(int i=0;i<bookmarks->count();i++) {
        bookmark=bookmarks->at(i);
        ui->bookmarksListWidget->addItem(bookmark->getTitle());
    }
}

BookmarksDialog::~BookmarksDialog()
{
    delete ui;
}

int BookmarksDialog::getSelected() {
    return ui->bookmarksListWidget->currentRow();
}
