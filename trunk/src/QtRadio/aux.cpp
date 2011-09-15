#include "aux.h"
#include "ui_aux.h"

Aux::Aux(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Aux)
{
    ui->setupUi(this);
}

Aux::~Aux()
{
    delete ui;
}
