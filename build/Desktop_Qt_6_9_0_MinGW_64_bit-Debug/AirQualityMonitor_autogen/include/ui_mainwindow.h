/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout_3;
    QLabel *statusLabel;
    QLabel *connectionStatusLabel;
    QPushButton *fetchDataButton;
    QComboBox *stationComboBox;
    QComboBox *sensorComboBox;
    QTextEdit *analysisTextEdit;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QLineEdit *dataAnalysisLineEdit;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(796, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayoutWidget = new QWidget(centralwidget);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(10, 10, 311, 511));
        verticalLayout_3 = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        statusLabel = new QLabel(verticalLayoutWidget);
        statusLabel->setObjectName("statusLabel");

        verticalLayout_3->addWidget(statusLabel);

        connectionStatusLabel = new QLabel(verticalLayoutWidget);
        connectionStatusLabel->setObjectName("connectionStatusLabel");

        verticalLayout_3->addWidget(connectionStatusLabel);

        fetchDataButton = new QPushButton(verticalLayoutWidget);
        fetchDataButton->setObjectName("fetchDataButton");

        verticalLayout_3->addWidget(fetchDataButton);

        stationComboBox = new QComboBox(verticalLayoutWidget);
        stationComboBox->setObjectName("stationComboBox");

        verticalLayout_3->addWidget(stationComboBox);

        sensorComboBox = new QComboBox(verticalLayoutWidget);
        sensorComboBox->setObjectName("sensorComboBox");

        verticalLayout_3->addWidget(sensorComboBox);

        analysisTextEdit = new QTextEdit(verticalLayoutWidget);
        analysisTextEdit->setObjectName("analysisTextEdit");

        verticalLayout_3->addWidget(analysisTextEdit);

        gridLayoutWidget = new QWidget(centralwidget);
        gridLayoutWidget->setObjectName("gridLayoutWidget");
        gridLayoutWidget->setGeometry(QRect(330, 10, 461, 511));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        dataAnalysisLineEdit = new QLineEdit(centralwidget);
        dataAnalysisLineEdit->setObjectName("dataAnalysisLineEdit");
        dataAnalysisLineEdit->setGeometry(QRect(10, 530, 781, 22));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 796, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        statusLabel->setText(QCoreApplication::translate("MainWindow", "Status: brak danych", nullptr));
        connectionStatusLabel->setText(QCoreApplication::translate("MainWindow", "Status po\305\202\304\205czenia", nullptr));
        fetchDataButton->setText(QCoreApplication::translate("MainWindow", "Pobierz dane", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
