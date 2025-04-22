#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    // chartView dynamicznie
    QChartView *chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);

    // Nadajemy nazwę chartView
    chartView->setObjectName("chartView");

    // Tworzymy wykres
    QChart *chart = new QChart();
    chartView->setChart(chart);

    // Dodajemy wykres go do istniejącego layoutu
    ui->gridLayout->addWidget(chartView);

    // Połącz sygnał odpowiedzi ze slotem
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &MainWindow::onNetworkReplyFinished);

    connect(ui->sensorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::on_sensorComboBox_currentIndexChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_fetchDataButton_clicked() {
    updateOnlineStatus();

    if (isInternetAvailable()) {
        // Pobieranie stacji
        QUrl url("https://api.gios.gov.pl/pjp-api/rest/station/findAll");
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::User, "stations");
        networkManager->get(request);

        // Pobieranie danych pomiarowych
        int sensorId = ui->sensorComboBox->currentData().toInt();
        if (sensorId != 0) {
            QUrl url1("https://api.gios.gov.pl/pjp-api/rest/data/getData/" + QString::number(sensorId));
            QNetworkRequest request1(url1);
            request1.setAttribute(QNetworkRequest::User, "measurements");
            networkManager->get(request1);
        }
    } else {
        // Tryb offline – ładowanie z pliku
        QByteArray data = loadDataFromFile("stations");
        if (!data.isEmpty()) {
            parseStationData(data);
        }

        int sensorId = ui->sensorComboBox->currentData().toInt();
        if (sensorId != 0) {
            QByteArray data1 = loadDataFromFile("measurements_" + QString::number(sensorId));
            if (!data1.isEmpty()) {
                parseMeasurementData(data1);
            }
        }
    }
}

void MainWindow::onNetworkReplyFinished(QNetworkReply *reply) {
    QByteArray responseData = reply->readAll();
    QString requestType = reply->request().attribute(QNetworkRequest::User).toString();

    if (requestType == "stations") {
        parseStationData(responseData);
        saveDataToFile("stations", responseData);
    } else if (requestType == "sensors") {
        parseSensorData(responseData);
        int stationId = ui->stationComboBox->currentData().toInt();
        saveDataToFile("sensors_" + QString::number(stationId), responseData);
    } else if (requestType == "measurements") {
        parseMeasurementData(responseData);
        int sensorId = ui->sensorComboBox->currentData().toInt();
        saveDataToFile("measurements_" + QString::number(sensorId), responseData);
    }

    reply->deleteLater();
}


void MainWindow::on_stationComboBox_currentIndexChanged(int index) {
    if (index < 0) return;

    int stationId = ui->stationComboBox->itemData(index).toInt();
    ui->statusLabel->setText("Pobieranie czujników dla stacji ID: " + QString::number(stationId));

    if (isInternetAvailable()) {
        QUrl url("https://api.gios.gov.pl/pjp-api/rest/station/sensors/" + QString::number(stationId));
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::User, "sensors");
        networkManager->get(request);
    } else {
        QByteArray data = loadDataFromFile("sensors_" + QString::number(stationId));
        if (!data.isEmpty()) {
            parseSensorData(data);
        } else {
            ui->statusLabel->setText("Brak zapisanych danych o czujnikach dla tej stacji.");
            ui->sensorComboBox->clear();
        }
    }
}


void MainWindow::on_sensorComboBox_currentIndexChanged(int index) {
    if (index < 0) return;

    int sensorId = ui->sensorComboBox->itemData(index).toInt();
    ui->statusLabel->setText("Pobieranie danych pomiarowych dla czujnika ID: " + QString::number(sensorId));

    if (isInternetAvailable()) {
        QUrl url("https://api.gios.gov.pl/pjp-api/rest/data/getData/" + QString::number(sensorId));
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::User, "measurements");
        networkManager->get(request);
    } else {
        QByteArray data = loadDataFromFile("measurements_" + QString::number(sensorId));
        if (!data.isEmpty()) {
            parseMeasurementData(data);
        } else {
            ui->statusLabel->setText("Brak zapisanych danych pomiarowych dla czujnika.");
            ui->analysisTextEdit->setPlainText("");
            QChartView *chartView = findChild<QChartView*>("chartView");
            if (chartView) {
                chartView->setChart(new QChart()); // czyści wykres
            }
        }
    }
}

void MainWindow::saveDataToFile(const QString &type, const QByteArray &data) {
    QString filename = QDir::currentPath() + "/" + type + ".json";
    QFile file(filename);

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(data);
        file.close();
    } else {
        qWarning("Nie udało się zapisać danych do pliku: %s", qPrintable(filename));
    }
}

QByteArray MainWindow::loadDataFromFile(const QString &type) {
    QString filename = QDir::currentPath() + "/" + type + ".json";
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray data = file.readAll();
        file.close();
        return data;
    }
    return QByteArray();
}

bool MainWindow::isInternetAvailable() {
    // Metoda do sprawdzenia dostępności internetu
    QNetworkRequest request(QUrl("http://www.google.com"));
    QNetworkReply *reply = networkManager->get(request);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(); // Czekaj na zakończenie odpowiedzi

    if (reply->error() == QNetworkReply::NoError) {
        return true;  // Internet dostępny
    } else {
        return false; // Internet niedostępny
    }
}

void MainWindow::updateOnlineStatus() {
    if (isInternetAvailable()) {
        ui->connectionStatusLabel->setText("Online");
    } else {
        ui->connectionStatusLabel->setText("Offline");
    }
}

void MainWindow::parseStationData(const QByteArray &data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return;

    ui->stationComboBox->clear();

    QJsonArray array = doc.array();
    for (const QJsonValue &val : array) {
        QJsonObject obj = val.toObject();
        int id = obj["id"].toInt();
        QString name = obj["stationName"].toString();
        ui->stationComboBox->addItem(name, id);
    }

    if (ui->stationComboBox->count() > 0)
        on_stationComboBox_currentIndexChanged(ui->stationComboBox->currentIndex());

}

void MainWindow::parseSensorData(const QByteArray &data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return;

    ui->sensorComboBox->clear();

    QJsonArray array = doc.array();
    for (const QJsonValue &val : array) {
        QJsonObject obj = val.toObject();
        int id = obj["id"].toInt();
        QString param = obj["param"].toObject()["paramName"].toString();
        ui->sensorComboBox->addItem(param, id);
    }
}

void MainWindow::parseMeasurementData(const QByteArray &data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;

    QJsonObject rootObj = doc.object();

    // Nazwa badanego parametru chemicznego
    QString paramName = rootObj["key"].toString();

    QJsonArray values = rootObj["values"].toArray();
    QLineSeries *series = new QLineSeries();
    series->setName(paramName);  // dodajemy nazwę do legendy

    QStringList analysisTextList;

    for (const QJsonValue &val : values) {
        QJsonObject obj = val.toObject();
        QString dateStr = obj["date"].toString();
        double value = obj["value"].toDouble(-1);

        if (value >= 0) {
            QDateTime timestamp = QDateTime::fromString(dateStr, Qt::ISODate);
            series->append(timestamp.toMSecsSinceEpoch(), value);

            // Linię tekstową z datą i wartością
            analysisTextList.append(timestamp.toString("dd.MM.yyyy HH:mm") + " → " + QString::number(value, 'f', 2));
        }
    }

    // Wykres
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Wykres stężenia: " + paramName);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // Oś X - czas
    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setFormat("dd.MM HH:mm");
    axisX->setTitleText("Data i czas");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Oś Y - wartość pomiaru
    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Wartość");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Znajdujemy dynamicznie utworzony chartView
    QChartView *chartView = findChild<QChartView*>("chartView");
    if (chartView) {
        chartView->setChart(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
    }

    // Wyświetlamy dane pomiarowe w analysisTextEdit
    ui->analysisTextEdit->setPlainText(analysisTextList.join("\n"));

    performDataAnalysis(values);
}

void MainWindow::performDataAnalysis(const QJsonArray &valuesArray) { // Analiza danych (max, min, avg, trend)
    if (valuesArray.isEmpty()) {
        ui->dataAnalysisLineEdit->setText("Brak danych do analizy.");
        return;
    }

    double sum = 0;
    int count = 0;
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();
    QString minTimeRaw, maxTimeRaw;

    QVector<double> values;
    for (const QJsonValue &val : valuesArray) {
        QJsonObject obj = val.toObject();
        if (obj["value"].isDouble() && !obj["value"].isNull()) {
            double v = obj["value"].toDouble();
            QString time = obj["date"].toString();
            values.append(v);
            sum += v;
            count++;
            if (v < minVal) {
                minVal = v;
                minTimeRaw = time;
            }
            if (v > maxVal) {
                maxVal = v;
                maxTimeRaw = time;
            }
        }
    }

    if (count == 0) {
        ui->dataAnalysisLineEdit->setText("Brak poprawnych danych do analizy.");
        return;
    }

    double avg = sum / count;
    QString trend = values.last() > values.first() ? "wzrostowy 📈" : "spadkowy 📉";

    // Formatowanie czasu
    QDateTime minTime = QDateTime::fromString(minTimeRaw, Qt::ISODate);
    QDateTime maxTime = QDateTime::fromString(maxTimeRaw, Qt::ISODate);
    QString minTimeStr = QString("dnia %1 o %2")
                             .arg(minTime.date().toString("dd.MM.yyyy"))
                             .arg(minTime.time().toString("HH:mm"));
    QString maxTimeStr = QString("dnia %1 o %2")
                             .arg(maxTime.date().toString("dd.MM.yyyy"))
                             .arg(maxTime.time().toString("HH:mm"));

    QString result;
    result += "🔽 Wartość najmniejsza: " + QString::number(minVal, 'f', 2) + " " + minTimeStr + ", ";
    result += "🔼 największa: " + QString::number(maxVal, 'f', 2) + " " + maxTimeStr + ", ";
    result += "📊 średnia: " + QString::number(avg, 'f', 2) + ", ";
    result += "Trend: " + trend;

    ui->dataAnalysisLineEdit->setText(result);
}
