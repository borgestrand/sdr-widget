#include "BookmarkDialog.h"

BookmarkDialog::BookmarkDialog() {
    widget.setupUi(this);
    widget.bandLineEdit->setEnabled(false);
    widget.frequencyLineEdit->setEnabled(false);
    widget.modeLineEdit->setEnabled(false);
    widget.filterLineEdit->setEnabled(false);
}

void BookmarkDialog::setTitle(QString t) {
    widget.titleLineEdit->setText(t);
}

void BookmarkDialog::setBand(QString b) {
    widget.bandLineEdit->setText(b);
}

void BookmarkDialog::setFrequency(QString f) {
    widget.frequencyLineEdit->setText(f);
}

void BookmarkDialog::setMode(QString m) {
    widget.modeLineEdit->setText(m);
}

void BookmarkDialog::setFilter(QString f) {
    widget.filterLineEdit->setText(f);
}

QString BookmarkDialog::getTitle() {
    return widget.titleLineEdit->text();
}
