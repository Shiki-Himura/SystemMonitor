![C++](https://img.shields.io/badge/C++-17%2F20-blue)
![Qt](https://img.shields.io/badge/Qt-6-green)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey)
![License](https://img.shields.io/badge/License-MIT-yellow)

\# SystemMonitor



Ein plattformübergreifendes System-Monitoring-Tool entwickelt mit modernem C++ und Qt6.



![SystemMonitor Screenshot](Screenshots/main.png?raw=true)



\## 🚀 Features



\- \*\*Echtzeit CPU-Überwachung\*\* - Live CPU-Auslastung mit historischen Graphen

\- \*\*Speicher-Tracking\*\* - RAM-Nutzung mit detaillierten Statistiken

\- \*\*Prozess-Management\*\* - Anzeige laufender Prozesse sortiert nach Speicherverbrauch

\- \*\*Cross-Platform\*\* - Funktioniert auf Windows und Linux

\- \*\*Moderne C++17/20\*\* - Nutzt moderne C++-Features und Best Practices

\- \*\*Interaktive Charts\*\* - Echtzeit-Datenvisualisierung mit QCustomPlot



\## 🛠️ Technologien



\- \*\*C++17/20\*\* - Moderne C++-Standards

\- \*\*Qt6\*\* - GUI-Framework (Core, Widgets, PrintSupport)

\- \*\*QCustomPlot\*\* - High-Performance Plotting-Library

\- \*\*CMake\*\* - Build-System

\- \*\*Platform APIs\*\* - Windows API (psapi) / Linux procfs



\## 📦 Installation



\### Voraussetzungen:

\- CMake 3.16+

\- Qt6 (Core, Widgets, PrintSupport)

\- QCustomPlot (im Projekt enthalten)

\- C++17 kompatibler Compiler (GCC 7+, Clang 5+, MSVC 2017+)



\### Windows:



1\. Qt6 installieren von \[qt.io](https://www.qt.io/download)

2\. QCustomPlot herunterladen:

```bash

\# Bereits im Projekt enthalten: qcustomplot.h und qcustomplot.cpp

```



3\. Build:

```cmd

mkdir build \&\& cd build

cmake -G "MinGW Makefiles" -DCMAKE\_PREFIX\_PATH="C:\\Qt\\6.7.0\\mingw\_64" ..

cmake --build . --config Release

```



4\. Starten:

```cmd

Release\\SystemMonitor.exe

```



\### Linux:

```bash

\# Qt6 installieren

sudo apt install qt6-base-dev libqt6printsupport6 cmake build-essential



\# Build

mkdir build \&\& cd build

cmake ..

make -j4

./SystemMonitor

```



\## 🏗️ Architektur



\- \*\*SystemInfo Class\*\* - Platform-spezifische Systemdaten-Erfassung

\- \*\*MonitorWindow\*\* - Haupt-UI mit QCustomPlot-Integration

\- \*\*Timer-basierte Updates\*\* - 1-Sekunden-Intervall für Echtzeit-Monitoring

\- \*\*Effizientes Daten-Management\*\* - Rolling Window für historische Daten (60 Sekunden)



\## 📸 Screenshots



![Hauptfenster](screenshots/main.png)

![CPU Graph](screenshots/cpu.png)

![Prozessliste](screenshots/processes.png)



\## 🎯 Zukünftige Features



\- \[ ] Disk-Nutzungs-Monitoring

\- \[ ] Netzwerk-Aktivitäts-Tracking

\- \[ ] CPU-per-Core-Aufschlüsselung

\- \[ ] Prozess-Beendigungs-Feature

\- \[ ] Export nach CSV

\- \[ ] Anpassbare Aktualisierungs-Intervalle

\- \[ ] Dark Mode Theme



\## 📄 Lizenz



MIT License - Frei verwendbar für eigene Projekte!



\## 👨‍💻 Autor



\*\*Alex Thaus\*\*

\- GitHub: \[@Shiki-Himura](https://github.com/Shiki-Himura)

\- Email: alex.thaus@black-sector.com



\## 🙏 Credits



\- \[QCustomPlot](https://www.qcustomplot.com/) - GPL v3 License

