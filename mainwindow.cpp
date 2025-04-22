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
    , networkManager(new QNetworkAccessManager(this))  // <--- dodaj to
{
    ui->setupUi(this);

    // Tworzymy chartView dynamicznie
    QChartView *chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);

    // Nadajemy nazwę chartView
    chartView->setObjectName("chartView"); // Nadajemy nazwę dla późniejszego odniesienia

    // Tworzymy wykres
    QChart *chart = new QChart();
    chartView->setChart(chart);

    // Ustawiamy chartView na głównym layoucie lub dodajemy go do istniejącego layoutu
    ui->gridLayout->addWidget(chartView);  // Przykład, jak dodać do layoutu

    // Połącz sygnał odpowiedzi z naszym slotem
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
    ui->statusLabel->setText("Pobieranie danych...");
    QUrl url("https://api.gios.gov.pl/pjp-api/rest/station/findAll");
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, "stations");
    networkManager->get(request);

    int sensorIndex = ui->sensorComboBox->currentIndex();
    if (sensorIndex < 0) {
        ui->statusLabel->setText("Nie wybrano czujnika.");
        return;
    }

    int sensorId = ui->sensorComboBox->itemData(sensorIndex).toInt();
    ui->statusLabel->setText("Pobieranie danych pomiarowych dla czujnika ID: " + QString::number(sensorId));

    QUrl url1("https://api.gios.gov.pl/pjp-api/rest/data/getData/" + QString::number(sensorId));
    QNetworkRequest request1(url1);
    request.setAttribute(QNetworkRequest::User, "measurements");
    networkManager->get(request);
}

void MainWindow::on_showChartButton_clicked() {
    // Tutaj będzie logika rysowania wykresu
}

void MainWindow::onNetworkReplyFinished(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        ui->statusLabel->setText("Błąd pobierania danych: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QVariant requestType = reply->request().attribute(QNetworkRequest::User);
    QString type = requestType.toString();

    if (type == "stations") {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isArray()) {
            ui->statusLabel->setText("Niepoprawny format danych (stations)");
            reply->deleteLater();
            return;
        }

        QJsonArray stationsArray = jsonDoc.array();
        QVariant selectedStationId = ui->stationComboBox->currentData();

        ui->stationComboBox->blockSignals(true);
        ui->stationComboBox->clear();

        for (const QJsonValue &stationValue : stationsArray) {
            QJsonObject stationObj = stationValue.toObject();
            QString stationName = stationObj["stationName"].toString();
            int stationId = stationObj["id"].toInt();
            ui->stationComboBox->addItem(stationName, stationId);
        }

        for (int i = 0; i < ui->stationComboBox->count(); ++i) {
            if (ui->stationComboBox->itemData(i) == selectedStationId) {
                ui->stationComboBox->setCurrentIndex(i);
                break;
            }
        }

        ui->stationComboBox->blockSignals(false);
        ui->statusLabel->setText("Załadowano " + QString::number(stationsArray.size()) + " stacji.");
    }
    else if (type == "sensors") {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isArray()) {
            ui->statusLabel->setText("Niepoprawny format danych (sensors)");
            reply->deleteLater();
            return;
        }

        QJsonArray sensorsArray = jsonDoc.array();
        QVariant selectedSensorId = ui->sensorComboBox->currentData();

        ui->sensorComboBox->blockSignals(true);
        ui->sensorComboBox->clear();

        for (const QJsonValue &sensorValue : sensorsArray) {
            QJsonObject sensorObj = sensorValue.toObject();
            QString paramName = sensorObj["param"].toObject()["paramName"].toString();
            int sensorId = sensorObj["id"].toInt();
            ui->sensorComboBox->addItem(paramName, sensorId);
        }

        for (int i = 0; i < ui->sensorComboBox->count(); ++i) {
            if (ui->sensorComboBox->itemData(i) == selectedSensorId) {
                ui->sensorComboBox->setCurrentIndex(i);
                break;
            }
        }

        ui->sensorComboBox->blockSignals(false);
        ui->statusLabel->setText("Załadowano " + QString::number(sensorsArray.size()) + " czujników.");
    }
    else if (type == "measurements") {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        if (!jsonDoc.isObject()) {
            ui->statusLabel->setText("Niepoprawny format danych (measurements)");
            reply->deleteLater();
            return;
        }

        QJsonObject obj = jsonDoc.object();
        QString paramName = obj["key"].toString();
        QJsonArray values = obj["values"].toArray();

        QString result = "Pomiar: " + paramName + "\n\n";

        QLineSeries *series = new QLineSeries();
        series->setName(paramName);

        for (const QJsonValue &val : values) {
            QJsonObject valObj = val.toObject();
            QString dateStr = valObj["date"].toString();
            QDateTime timestamp = QDateTime::fromString(dateStr, Qt::ISODate);
            if (!timestamp.isValid()) continue;

            if (!valObj["value"].isNull()) {
                double value = valObj["value"].toDouble();
                series->append(timestamp.toMSecsSinceEpoch(), value);
                result += timestamp.toString("yyyy-MM-dd HH:mm") + ": " + QString::number(value) + "\n";
            } else {
                result += timestamp.toString("yyyy-MM-dd HH:mm") + ": brak danych\n";
            }
        }

        ui->analysisTextEdit->setPlainText(result);
        ui->statusLabel->setText("Dane pomiarowe załadowane.");

        // Tworzenie wykresu
        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Wykres pomiarów: " + paramName);

        QDateTimeAxis *axisX = new QDateTimeAxis;
        axisX->setFormat("dd.MM HH:mm");
        axisX->setTitleText("Data i czas");
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis;
        axisY->setTitleText("Wartość");
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        // Znajdowanie dynamicznie dodanego chartView po nazwie
        QChartView *chartView = findChild<QChartView*>("chartView");

        if (chartView) {
            chartView->setChart(chart);
            chartView->setRenderHint(QPainter::Antialiasing);
        } else {
            ui->statusLabel->setText("Nie znaleziono widoku wykresu.");
        }
    }
    else {
        ui->statusLabel->setText("Nieznany typ odpowiedzi");
    }

    reply->deleteLater();
}

void MainWindow::on_stationComboBox_currentIndexChanged(int index) {
    if (index < 0) return;

    int stationId = ui->stationComboBox->itemData(index).toInt();

    ui->statusLabel->setText("Pobieranie czujników dla stacji ID: " + QString::number(stationId));

    QUrl url("https://api.gios.gov.pl/pjp-api/rest/station/sensors/" + QString::number(stationId));
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, "sensors");
    networkManager->get(request);
}

void MainWindow::on_sensorComboBox_currentIndexChanged(int index) {
    if (index < 0) return;

    int sensorId = ui->sensorComboBox->itemData(index).toInt();
    ui->statusLabel->setText("Pobieranie danych pomiarowych dla czujnika ID: " + QString::number(sensorId));

    QUrl url("https://api.gios.gov.pl/pjp-api/rest/data/getData/" + QString::number(sensorId));
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, "measurements");
    networkManager->get(request);
}
