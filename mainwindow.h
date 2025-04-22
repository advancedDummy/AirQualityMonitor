#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;

private slots:
    void on_fetchDataButton_clicked();
    void on_showChartButton_clicked();
    void onNetworkReplyFinished(QNetworkReply* reply);
    void on_stationComboBox_currentIndexChanged(int index);
    void on_sensorComboBox_currentIndexChanged(int index);
};
#endif // MAINWINDOW_H
