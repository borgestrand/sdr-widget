#include "ctl.h"
#include "ui_ctl.h"

Ctl::Ctl(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Ctl)
{
    ui->setupUi(this);
}

Ctl::~Ctl()
{
    delete ui;
}
