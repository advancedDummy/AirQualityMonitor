#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(new QNetworkAccessManager(this))  // <--- dodaj to
{
    ui->setupUi(this);

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

        QVariant selectedStationId = ui->stationComboBox->currentData(); // zapamiętaj wybraną stację

        ui->stationComboBox->blockSignals(true); // wyłącz sygnały, by nie odpalać currentIndexChanged
        ui->stationComboBox->clear();

        for (const QJsonValue &stationValue : stationsArray) {
            QJsonObject stationObj = stationValue.toObject();
            QString stationName = stationObj["stationName"].toString();
            int stationId = stationObj["id"].toInt();
            ui->stationComboBox->addItem(stationName, stationId);
        }

        // Przywróć wybór, jeśli się da
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

        QVariant selectedSensorId = ui->sensorComboBox->currentData(); // zapamiętaj wybrany czujnik

        ui->sensorComboBox->blockSignals(true);
        ui->sensorComboBox->clear();

        for (const QJsonValue &sensorValue : sensorsArray) {
            QJsonObject sensorObj = sensorValue.toObject();
            QString paramName = sensorObj["param"].toObject()["paramName"].toString();
            int sensorId = sensorObj["id"].toInt();
            ui->sensorComboBox->addItem(paramName, sensorId);
        }

        // Przywróć wybór, jeśli się da
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

        for (const QJsonValue &val : values) {
            QJsonObject valObj = val.toObject();
            QString date = valObj["date"].toString();
            QString valueStr = valObj["value"].isNull() ? "brak danych" : QString::number(valObj["value"].toDouble());
            result += date + ": " + valueStr + "\n";
        }

        ui->analysisTextEdit->setPlainText(result);
        ui->statusLabel->setText("Dane pomiarowe załadowane.");
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
