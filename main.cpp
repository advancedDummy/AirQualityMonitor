/**
 * @file main.cpp
 * @brief Główny plik uruchamiający aplikację.
 *
 * Ten plik zawiera kod inicjujący aplikację Qt oraz uruchamiający główne okno aplikacji.
 */

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
