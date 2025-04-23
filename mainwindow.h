/**
 * @file mainwindow.h
 * @brief Nagłówek klasy MainWindow.
 *
 * Plik ten zawiera deklarację klasy MainWindow, która zarządza głównym interfejsem
 * graficznym aplikacji. UI definiowany jest w pliku `mainwindow.ui`.
 * Zawiera funkcje umożliwiające interakcję z API GIOŚ, zarządzanie stanem połączenia,
 * oraz obsługę zdarzeń związanych z interfejsem użytkownika.
 *
 * Widżety interfejsu:
 * - QLabel `statusLabel`: Wyświetla status aplikacji ("brak danych").
 * - QLabel `connectionStatusLabel`: Pokazuje status połączenia (API/internet).
 * - QPushButton `fetchDataButton`: Przycisk do pobierania danych z API.
 * - QComboBox `stationComboBox`, `sensorComboBox`: Wybór stacji i sensora.
 * - QTextEdit `analysisTextEdit`: Wyświetlanie wyników analizy.
 * - QLineEdit `dataAnalysisLineEdit`: Pole analizy w formie liniowej.
 * - QMenuBar, QStatusBar: Standardowe paski menu i statusu.
 */

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

/**
 * @class MainWindow
 * @brief Klasa reprezentująca główne okno aplikacji.
 *
 * Klasa MainWindow zawiera wszystkie elementy interfejsu użytkownika oraz logikę aplikacji,
 * w tym połączenia z API GIOŚ, pobieranie danych o stacjach pomiarowych i czujnikach,
 * a także wyświetlanie wyników na wykresach.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy MainWindow.
     *
     * Inicjalizuje interfejs użytkownika oraz menedżer sieciowy do obsługi zapytań HTTP.
     *
     * @param parent Opcjonalny wskaźnik do rodzica (domyślnie nullptr).
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy MainWindow.
     *
     * Zwolnienie zasobów przy zamknięciu okna aplikacji.
     */
    ~MainWindow();

private:
    Ui::MainWindow *ui; /**< Wskaźnik do interfejsu użytkownika */
    QNetworkAccessManager *networkManager; /**< Menedżer sieciowy do obsługi zapytań HTTP */

    /**
     * @brief Zapisuje dane do pliku JSON.
     *
     * Zapisuje dane do pliku w bieżącym katalogu.
     *
     * @param type Typ danych (np. "stations", "measurements").
     * @param data Dane do zapisania.
     */
    void saveDataToFile(const QString &type, const QByteArray &data);

    /**
     * @brief Wczytuje dane z pliku JSON.
     *
     * Odczytuje dane z pliku o podanej nazwie.
     *
     * @param type Typ danych do załadowania (np. "stations", "measurements").
     * @return Wczytane dane w formacie QByteArray.
     */
    QByteArray loadDataFromFile(const QString &type);

    /**
     * @brief Sprawdza dostępność internetu.
     *
     * Wysyła zapytanie HTTP do serwera Google, aby sprawdzić, czy połączenie internetowe jest dostępne.
     *
     * @return true, jeśli internet jest dostępny, w przeciwnym razie false.
     */
    bool isInternetAvailable();

    /**
     * @brief Aktualizuje status połączenia w interfejsie użytkownika.
     *
     * Zmienia tekst etykiety statusu połączenia w zależności od dostępności internetu i API.
     */
    void updateOnlineStatus();

    /**
     * @brief Przetwarza dane stacji pomiarowej.
     *
     * Analizuje dane stacji pomiarowej w formacie JSON i aktualizuje interfejs użytkownika.
     *
     * @param data Dane stacji w formacie QByteArray.
     */
    void parseStationData(const QByteArray &data);

    /**
     * @brief Przetwarza dane czujników.
     *
     * Analizuje dane czujników w formacie JSON i aktualizuje interfejs użytkownika.
     *
     * @param data Dane czujników w formacie QByteArray.
     */
    void parseSensorData(const QByteArray &data);

    /**
     * @brief Przetwarza dane pomiarowe.
     *
     * Analizuje dane pomiarowe w formacie JSON i wyświetla je na wykresie.
     *
     * @param data Dane pomiarowe w formacie QByteArray.
     */
    void parseMeasurementData(const QByteArray &data);

    /**
     * @brief Przeprowadza analizę danych.
     *
     * Analizuje dane pomiarowe z formatu JSON i wyświetla wyniki analizy w interfejsie użytkownika.
     *
     * @param valuesArray Tablica danych pomiarowych w formacie QJsonArray.
     */
    void performDataAnalysis(const QJsonArray &valuesArray);

    /**
     * @brief Sprawdza dostępność API GIOŚ.
     *
     * Wysyła zapytanie HTTP do API GIOŚ, aby sprawdzić, czy jest ono dostępne.
     *
     * @return true, jeśli API GIOŚ jest dostępne, w przeciwnym razie false.
     */
    bool isApiAvailable();

private slots:
    /**
     * @brief Obsługuje kliknięcie przycisku "Pobierz dane".
     *
     * Wywołuje zapytania sieciowe w celu pobrania danych o stacjach i czujnikach.
     */
    void on_fetchDataButton_clicked();

    /**
     * @brief Obsługuje odpowiedź na zapytanie sieciowe.
     *
     * Przetwarza odpowiedź sieciową i aktualizuje dane w interfejsie użytkownika.
     *
     * @param reply Odpowiedź z serwera.
     */
    void onNetworkReplyFinished(QNetworkReply* reply);

    /**
     * @brief Obsługuje zmianę wybranej stacji.
     *
     * Pobiera dane czujników dla wybranej stacji.
     *
     * @param index Indeks wybranej stacji.
     */
    void on_stationComboBox_currentIndexChanged(int index);

    /**
     * @brief Obsługuje zmianę wybranego czujnika.
     *
     * Pobiera dane pomiarowe dla wybranego czujnika.
     *
     * @param index Indeks wybranego czujnika.
     */
    void on_sensorComboBox_currentIndexChanged(int index);
};
#endif // MAINWINDOW_H
