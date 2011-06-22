/*
 * File:   main.cpp
 * Author: john
 *
 * Created on 05 August 2010, 12:00
 */

#include <QtGui/QApplication>
#include "qtMonitor.h"

int main(int argc, char *argv[]) {
    // initialize resources, if needed
    // Q_INIT_RESOURCE(resfile);

    QApplication app(argc, argv);

    // create and show your widgets here
    qtMonitor mainWindow;

    mainWindow.show();

    return app.exec();
}
