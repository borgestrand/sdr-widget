#include "BookmarksEditDialog.h"
#include "ui_BookmarksEditDialog.h"

BookmarksEditDialog::BookmarksEditDialog(QWidget *parent,Bookmarks* bookmarks) :
    QDialog(parent),
    ui(new Ui::BookmarksEditDialog)
{
    ui->setupUi(this);

    ui->bandLineEdit->setEnabled(false);
    ui->frequencyLineEdit->setEnabled(false);
    ui->modeLineEdit->setEnabled(false);
    ui->filterLineEdit->setEnabled(false);

    // load the bookmarks
    Bookmark* bookmark;
    for(int i=0;i<bookmarks->count();i++) {
        bookmark=bookmarks->at(i);

        ui->bookmarksListWidget->addItem(bookmark->getTitle());
    }

    connect(ui->deletePushButton,SIGNAL(clicked()),this,SLOT(deleteBookmark()));
    connect(ui->updatePushButton,SIGNAL(clicked()),this,SLOT(updateBookmark()));
    connect(ui->bookmarksListWidget,SIGNAL(currentRowChanged(int)),this,SLOT(bookmarkRowChanged(int)));

}

BookmarksEditDialog::~BookmarksEditDialog()
{
    delete ui;
}

void BookmarksEditDialog::deleteBookmark() {
    int entry=ui->bookmarksListWidget->currentRow();

    if(entry>=0) {
        //qDebug() << "deleteBookmark:" << entry;
        //let other know it has been deleted
        emit bookmarkDeleted(entry);
        // remove from dialog list
        ui->bookmarksListWidget->takeItem(entry);
    }

}

void BookmarksEditDialog::updateBookmark() {
    int entry=ui->bookmarksListWidget->currentRow();

    if(entry>=0) {
        //qDebug() << "deleteBookmark:" << entry;
        //let other know it has been deleted
        QString title=ui->titleLineEdit->text();
        emit bookmarkUpdated(entry,title);

        ui->bookmarksListWidget->takeItem(entry);
        ui->bookmarksListWidget->addItem(title);
        ui->bookmarksListWidget->setCurrentRow(entry);

    }

}


void BookmarksEditDialog::bookmarkRowChanged(int row) {
    emit bookmarkSelected(row);
}

void BookmarksEditDialog::setTitle(QString t) {
    ui->titleLineEdit->setText(t);
}

void BookmarksEditDialog::setBand(QString b) {
    ui->bandLineEdit->setText(b);
}

void BookmarksEditDialog::setFrequency(QString f) {
    ui->frequencyLineEdit->setText(f);
}

void BookmarksEditDialog::setMode(QString m) {
    ui->modeLineEdit->setText(m);
}

void BookmarksEditDialog::setFilter(QString f) {
    ui->filterLineEdit->setText(f);
}
