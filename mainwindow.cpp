/**
 * @file mainwindow.cpp
 * @brief Definicje metod klasy MainWindow.
 *
 * Ten plik zawiera implementację głównego okna aplikacji monitorującej jakość powietrza
 * z wykorzystaniem danych z API GIOŚ. Zawiera funkcje odpowiedzialne za:
 * - interakcję z użytkownikiem,
 * - komunikację sieciową (tryb online i offline),
 * - przetwarzanie oraz wizualizację danych pomiarowych.
 *
 * @author Miłosz Wybrański
 * @date 22.04.2025
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QDateTime>

/**
 * @brief Konstruktor klasy MainWindow
 *
 * Inicjalizuje interfejs użytkownika oraz konfiguracje związane z menedżerem sieciowym.
 * Tworzy wykres, ustawia antyaliasing oraz dodaje go do layoutu.
 *
 * - `statusLabel`: Pokazuje status działania aplikacji.
 * - `connectionStatusLabel`: Informuje o połączeniu z internetem/API.
 * - `fetchDataButton`: Inicjuje pobieranie danych.
 * - `stationComboBox` / `sensorComboBox`: Wybór lokalizacji i sensora.
 * - `analysisTextEdit`: Pokazuje analizę danych w formie tekstowej.
 * - `dataAnalysisLineEdit`: Może wyświetlać pojedynczą wartość analizy.
 * @param parent Wskaźnik do rodzica okna głównego
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(new QNetworkAccessManager(this)) // Inicjalizacja menedżera sieciowego
{
    ui->setupUi(this);

    // Tworzenie obiektu wykresu i przypisanie antyaliasingu
    QChartView *chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);

    // Nadanie obiektowi wykresu unikalnej nazwy
    chartView->setObjectName("chartView");

    // Tworzenie wykresu i przypisanie go do chartView
    QChart *chart = new QChart();
    chartView->setChart(chart);

    // Dodanie wykresu do layoutu w interfejsie
    ui->gridLayout->addWidget(chartView);

    // Połączenie sygnału zakończenia zapytania sieciowego z odpowiednim slotem
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &MainWindow::onNetworkReplyFinished);

    // Połączenie zmiany wybranego czujnika z odpowiednim slotem
    connect(ui->sensorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::on_sensorComboBox_currentIndexChanged);
}

/**
 * @brief Destruktor klasy MainWindow
 *
 * Zwalnia zasoby związane z interfejsem użytkownika.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Obsługuje kliknięcie przycisku "Pobierz dane"
 *
 * Sprawdza dostępność internetu i API GIOŚ. W zależności od dostępności pobiera dane z sieci lub z plików lokalnych.
 */
void MainWindow::on_fetchDataButton_clicked() {
    updateOnlineStatus();

    if (isInternetAvailable()) {
        // Pobieranie listy stacji
        QUrl url("https://api.gios.gov.pl/pjp-api/rest/station/findAll");
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::User, "stations");
        networkManager->get(request);

        // Pobieranie danych z wybranego czujnika
        int sensorId = ui->sensorComboBox->currentData().toInt();
        if (sensorId != 0) {
            QUrl url1("https://api.gios.gov.pl/pjp-api/rest/data/getData/" + QString::number(sensorId));
            QNetworkRequest request1(url1);
            request1.setAttribute(QNetworkRequest::User, "measurements");
            networkManager->get(request1);
        }
    } else {
        // Tryb offline – odczyt danych z plików lokalnych
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

/**
 * @brief Obsługuje odpowiedź z zapytania sieciowego
 *
 * Przetwarza odpowiedź na podstawie typu zapytania (stacje, czujniki, pomiary),
 * zapisuje dane do plików lokalnych i wywołuje odpowiednią funkcję do analizy danych.
 *
 * @param reply Odpowiedź na zapytanie sieciowe
 */
void MainWindow::onNetworkReplyFinished(QNetworkReply *reply) {
    QByteArray responseData = reply->readAll();
    QString requestType = reply->request().attribute(QNetworkRequest::User).toString();

    // Przetwarzanie odpowiedzi w zależności od typu zapytania
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

    reply->deleteLater(); // Zwolnienie pamięci
}

/**
 * @brief Zmienia wybraną stację i pobiera dane czujników
 *
 * Przy zmianie wybranej stacji pobiera dane czujników stacji z internetu lub z plików lokalnych,
 * jeżeli tryb offline jest aktywowany.
 *
 * @param index Indeks wybranej stacji
 */
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

/**
 * @brief Zmienia wybranego czujnika i pobiera dane pomiarowe
 *
 * Przy zmianie wybranego czujnika pobiera dane pomiarowe z internetu lub z plików lokalnych,
 * w zależności od dostępności połączenia internetowego.
 *
 * @param index Indeks wybranego czujnika
 */
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

            // Czyszczenie wykresu, jeśli dane są niedostępne
            QChartView *chartView = findChild<QChartView*>("chartView");
            if (chartView) {
                chartView->setChart(new QChart()); // czyści wykres
            }
        }
    }
}

/**
 * @brief Zapisuje dane do pliku JSON
 *
 * Zapisuje dane w formacie JSON do pliku w bieżącym katalogu.
 *
 * @param type Typ danych (np. "stations", "measurements")
 * @param data Dane do zapisania
 */
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

/**
 * @brief Odczytuje dane z pliku JSON
 *
 * Odczytuje dane z pliku JSON i zwraca je jako QByteArray.
 *
 * @param type Typ danych do odczytania (np. "stations", "measurements")
 * @return QByteArray Zawartość pliku
 */
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

/**
 * @brief Sprawdza dostępność API GIOŚ.
 *
 * Funkcja wykonuje zapytanie do API GIOŚ, aby sprawdzić, czy odpowiedź na zapytanie
 * jest poprawna, co oznacza dostępność serwera API. Zwraca wartość typu bool, która
 * wskazuje, czy API jest dostępne.
 *
 * @return true, jeśli API jest dostępne (brak błędów w odpowiedzi), w przeciwnym przypadku false.
 */
bool MainWindow::isApiAvailable() {
    QNetworkRequest request(QUrl("https://api.gios.gov.pl/pjp-api/rest/station/findAll"));
    QNetworkReply *reply = networkManager->get(request);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    bool success = (reply->error() == QNetworkReply::NoError);
    reply->deleteLater();
    return success;
}

/**
 * @brief Sprawdza dostępność połączenia internetowego.
 *
 * Funkcja wysyła zapytanie do serwisu Google, aby sprawdzić, czy urządzenie ma dostęp
 * do internetu. Czekając na odpowiedź z serwera, sprawdza, czy wystąpił błąd w odpowiedzi.
 * Zwraca wartość typu bool, która wskazuje, czy połączenie internetowe jest dostępne.
 *
 * @return true, jeśli internet jest dostępny (brak błędów w odpowiedzi), w przeciwnym przypadku false.
 */
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

/**
 * @brief Aktualizuje status połączenia w interfejsie użytkownika.
 *
 * Funkcja sprawdza dostępność internetu i dostępność API GIOŚ. Na podstawie tych
 * informacji aktualizuje tekst i kolor etykiety statusu połączenia w interfejsie użytkownika.
 * Etykieta zmienia kolor na zielony, pomarańczowy lub czerwony w zależności od stanu połączenia.
 */
void MainWindow::updateOnlineStatus() {
    if (isInternetAvailable()) {
        if (isApiAvailable()) {
            ui->connectionStatusLabel->setText("Online (strona dostępna)");
            ui->connectionStatusLabel->setStyleSheet("QLabel { color : green; }");
        } else {
            ui->connectionStatusLabel->setText("Online (strona niedostępna)");
            ui->connectionStatusLabel->setStyleSheet("QLabel { color : orange; }");
        }
    } else {
        ui->connectionStatusLabel->setText("Offline (brak internetu)");
        ui->connectionStatusLabel->setStyleSheet("QLabel { color : red; }");
    }
}

/**
 * @brief Parsuje dane stacji i dodaje je do ComboBox'a.
 *
 * Funkcja odbiera dane w formacie JSON, konwertuje je na tablicę, a następnie
 * dodaje nazwy stacji do listy w ComboBoxie, przypisując jednocześnie ich identyfikatory
 * jako dane powiązane z elementami listy.
 *
 * @param data Dane stacji w formacie JSON.
 */
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

/**
 * @brief Parsuje dane czujników i dodaje je do ComboBox'a.
 *
 * Funkcja przetwarza dane czujników w formacie JSON, wydobywa nazwę parametru
 * i dodaje je do listy w ComboBoxie, przypisując odpowiednie identyfikatory
 * czujników jako dane powiązane z elementami listy.
 *
 * @param data Dane czujników w formacie JSON.
 */
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

/**
 * @brief Parsuje dane pomiarowe i wyświetla je na wykresie oraz w analizie tekstowej.
 *
 * Funkcja odbiera dane pomiarowe w formacie JSON, konwertuje je na odpowiednie
 * obiekty i rysuje wykres na podstawie wartości pomiarów. Ponadto generuje
 * analizę tekstową wyników, którą wyświetla w oknie aplikacji.
 *
 * @param data Dane pomiarowe w formacie JSON.
 */
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

            // Linia tekstowa z datą i wartością
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

/**
 * @brief Wykonuje analizę danych pomiarowych (maksimum, minimum, średnia, trend).
 *
 * Funkcja przetwarza dane pomiarowe, oblicza minimalną, maksymalną wartość,
 * średnią oraz wykrywa trend w danych (wzrostowy/spadkowy). Wyniki są
 * wyświetlane w interfejsie użytkownika.
 *
 * @param valuesArray Tablica wartości pomiarowych do analizy.
 */
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
