Air Quality Monitor – Qt C++ Application
========================================

Opis
-----
Aplikacja "Air Quality Monitor" służy do monitorowania jakości powietrza w Polsce na podstawie danych udostępnianych przez Główny Inspektorat Ochrony Środowiska (GIOŚ) poprzez publiczne REST API.

Umożliwia:
- Pobieranie danych pomiarowych z wybranych stacji w czasie rzeczywistym
- Przeglądanie wykresów
- Pracę w trybie offline z wykorzystaniem lokalnie zapisanych danych
- Wybór stacji i czujników z poziomu interfejsu graficznego

Funkcje
-------
- Pobieranie listy stacji i czujników z API GIOŚ
- Automatyczne zapisywanie odpowiedzi API do plików JSON (cache offline)
- Obsługa trybu offline (gdy brak internetu)
- Wizualizacja danych pomiarowych z użyciem wykresów

Wymagania
---------
- Qt 6.x z Qt Creator i Qt Design Studio (zalecane)
- C++
- Połączenie internetowe (dla trybu online)
- Dostęp do API GIOŚ (publiczny endpoint)

Struktura projektu
------------------
- `/src` – kod źródłowy aplikacji
- `/data` – zapisane odpowiedzi JSON do pracy offline
- `/doc` – dokumentacja wygenerowana przez Doxygen
- `README.txt` – ten plik

Uruchomienie
------------
1. Otwórz projekt w Qt Creator.
2. Skompiluj projekt (`Ctrl+B`).
3. Uruchom aplikację (`Ctrl+R`).

Działanie w trybie offline:
- Jeśli brak internetu, aplikacja użyje zapisanych lokalnie plików JSON zawierających dane z ostatnio przeglądanych czujników
- Dane te muszą być wcześniej zapisane podczas pracy online.

Dokumentacja
------------
Dokumentacja kodu źródłowego jest dostępna w folderze `/doc` w formacie HTML w pliku `index.html`, wygenerowana przy użyciu Doxygen.

Autor
-----
Projekt stworzony na potrzeby nauki i przetwarzania danych z REST API przez Miłosza Wybrańskiego.

