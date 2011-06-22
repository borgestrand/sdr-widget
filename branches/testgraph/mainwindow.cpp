#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QList>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //int gvht = ui->graphicsView->height();
    //int gvwt = ui->graphicsView->width();
    ui->graphicsView->setAlignment(Qt::AlignLeft|Qt::AlignTop );
    QGraphicsScene *sc = new QGraphicsScene();

    qreal gvht = (qreal)this->height();
    qreal gvwt = (qreal)this->width();
    qreal htrat = gvht/512.0;
    qreal wtrat = gvwt/512.0;
    qDebug() << "View" << gvwt << " " << gvht;
    qDebug() << "ratios" << wtrat << " " << htrat;

    QPixmap *pix = new QPixmap(512,1);
    pix->fill(Qt::blue);

    QPixmap *pix1 = new QPixmap(512,1);
    pix1->fill(Qt::red);


    QList<QPixmap *> waterfall;
    waterfall << pix;
    waterfall << pix;
    waterfall << pix;
    waterfall << pix;
    waterfall << pix1;
    waterfall << pix;
    waterfall << pix;
    waterfall << pix;
    waterfall << pix;
    waterfall << pix;
    waterfall << pix;
    waterfall << pix1;
    waterfall << pix;
    waterfall << pix;
    waterfall << pix;
    waterfall << pix;
    waterfall << pix;
    waterfall << pix;
    waterfall << pix1;
    waterfall << pix;
    waterfall << pix;


    for( int i=0; i<waterfall.size(); ++i)
    {
        sc->addPixmap(*waterfall[i])->setOffset(0,i);
    }


    waterfall.removeLast();
    waterfall.prepend(pix1);

    for( int i=0; i<waterfall.size(); ++i)
    {
        sc->addPixmap(*waterfall[i])->setOffset(0,i);
    }

    ui->graphicsView->setScene(sc);
    ui->graphicsView->show();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
