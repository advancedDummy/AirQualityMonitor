#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>

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
    void saveDataToFile(const QString &type, const QByteArray &data);
    QByteArray loadDataFromFile(const QString &type);
    bool isInternetAvailable();
    void updateOnlineStatus();
    void parseStationData(const QByteArray &data);
    void parseSensorData(const QByteArray &data);
    void parseMeasurementData(const QByteArray &data);
    void performDataAnalysis(const QJsonArray &valuesArray);

private slots:
    void on_fetchDataButton_clicked();
    void onNetworkReplyFinished(QNetworkReply* reply);
    void on_stationComboBox_currentIndexChanged(int index);
    void on_sensorComboBox_currentIndexChanged(int index);
};
#endif // MAINWINDOW_H
