#include "About.h"
#include "ui_About.h"

#include <QString>

#if !defined GITREV
#define GITREV "."
#endif

static const char git [] = GITREV;

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    ui->textBrowserGit->setHtml(QString(git).replace('!', "<br>") );   
}

About::~About()
{
    delete ui;
}
