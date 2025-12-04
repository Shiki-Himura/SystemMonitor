// SystemMonitor - Windows System Monitoring Tool
// Author: Alex Thaus
// C++17 | Qt6 | QCustomPlot
// Kompiliert für Windows mit Qt

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QTimer>
#include "qcustomplot.h"
#include <vector>
#include <string>
#include <algorithm>

// Windows Headers
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "psapi.lib")

// ===== System Info Collector (Windows) =====
class SystemInfo {
public:
    // CPU Usage Berechnung
    static double getCpuUsage() {
        static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
        static int numProcessors = 0;
        static bool first = true;


        if (first) {
            SYSTEM_INFO sysInfo;
            GetSystemInfo(&sysInfo);
            numProcessors = sysInfo.dwNumberOfProcessors;

            FILETIME ftime, fsys, fuser;
            GetSystemTimeAsFileTime(&ftime);
            memcpy(&lastCPU, &ftime, sizeof(FILETIME));

            HANDLE self = GetCurrentProcess();
            GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
            memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
            memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
            first = false;
            return 0.0;
        }

        FILETIME ftime, fsys, fuser;
        ULARGE_INTEGER now, sys, user;

        GetSystemTimeAsFileTime(&ftime);
        memcpy(&now, &ftime, sizeof(FILETIME));

        HANDLE self = GetCurrentProcess();
        GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
        memcpy(&sys, &fsys, sizeof(FILETIME));
        memcpy(&user, &fuser, sizeof(FILETIME));

        double percent = (sys.QuadPart - lastSysCPU.QuadPart) +
            (user.QuadPart - lastUserCPU.QuadPart);
        percent /= (now.QuadPart - lastCPU.QuadPart);
        percent /= numProcessors;
        lastCPU = now;
        lastUserCPU = user;
        lastSysCPU = sys;

        return percent * 100.0;
    }

    // Memory Usage (in MB)
    static std::pair<double, double> getMemoryUsage() {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        
        double totalMB = memInfo.ullTotalPhys / (1024.0 * 1024.0);
        double usedMB = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024.0 * 1024.0);
        return {usedMB, totalMB};
    }

    // Process Information
    struct ProcessInfo {
        std::string name;
        unsigned long pid;
        double memoryMB;
    };

    // Get list of all running processes
    static std::vector<ProcessInfo> getProcessList() {
        std::vector<ProcessInfo> processes;
        
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) 
            return processes;

        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(snapshot, &entry)) {
            do {
                HANDLE hProcess = OpenProcess(
                    PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
                    FALSE, 
                    entry.th32ProcessID
                );
                
                if (hProcess) {
                    PROCESS_MEMORY_COUNTERS pmc;
                    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                        ProcessInfo info;
                        info.name = QString::fromWCharArray(entry.szExeFile).toStdString();
                        info.pid = entry.th32ProcessID;
                        info.memoryMB = pmc.WorkingSetSize / (1024.0 * 1024.0);
                        processes.push_back(info);
                    }
                    CloseHandle(hProcess);
                }
            } while (Process32NextW(snapshot, &entry));
        }
        CloseHandle(snapshot);
        
        return processes;
    }
};

// ===== Main Window =====
class MonitorWindow : public QMainWindow {
    Q_OBJECT

private:
    QTimer* updateTimer;
    QCustomPlot* customPlot;
    QTableWidget* processTable;
    QLabel* cpuLabel;
    QLabel* memLabel;
    QLabel* statusLabel;
    
    QVector<double> timeData;
    QVector<double> cpuData;
    QVector<double> memData;
    
    const int maxDataPoints = 60;
    double currentTime = 0.0;

public:
    MonitorWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("SystemMonitor - by Alex Thaus");
        setMinimumSize(1000, 700);

        QWidget* central = new QWidget(this);
        setCentralWidget(central);
        QVBoxLayout* mainLayout = new QVBoxLayout(central);
        mainLayout->setSpacing(10);
        mainLayout->setContentsMargins(15, 15, 15, 15);

        // === Header mit Info Labels ===
        QHBoxLayout* infoLayout = new QHBoxLayout();
        
        cpuLabel = new QLabel("CPU: 0.0%");
        cpuLabel->setStyleSheet(
            "font-size: 16px; font-weight: bold; color: #2196F3; "
            "padding: 10px; background: #E3F2FD; border-radius: 5px;"
        );
        
        memLabel = new QLabel("RAM: 0 MB / 0 MB");
        memLabel->setStyleSheet(
            "font-size: 16px; font-weight: bold; color: #4CAF50; "
            "padding: 10px; background: #E8F5E9; border-radius: 5px;"
        );
        
        infoLayout->addWidget(cpuLabel);
        infoLayout->addWidget(memLabel);
        infoLayout->addStretch();
        mainLayout->addLayout(infoLayout);

        // === QCustomPlot Setup ===
        customPlot = new QCustomPlot();
        customPlot->setMinimumHeight(280);
        
        // Graph 0: CPU
        customPlot->addGraph();
        customPlot->graph(0)->setPen(QPen(QColor(33, 150, 243), 2));
        customPlot->graph(0)->setName("CPU %");
        customPlot->graph(0)->setBrush(QBrush(QColor(33, 150, 243, 30)));
        
        // Graph 1: Memory
        customPlot->addGraph();
        customPlot->graph(1)->setPen(QPen(QColor(76, 175, 80), 2));
        customPlot->graph(1)->setName("RAM %");
        customPlot->graph(1)->setBrush(QBrush(QColor(76, 175, 80, 30)));
        
        // Axes
        customPlot->xAxis->setLabel("Zeit (Sekunden)");
        customPlot->yAxis->setLabel("Auslastung (%)");
        customPlot->xAxis->setRange(0, maxDataPoints);
        customPlot->yAxis->setRange(0, 100);
        
        // Legend
        customPlot->legend->setVisible(true);
        customPlot->legend->setFont(QFont("Segoe UI", 9));
        customPlot->legend->setBrush(QBrush(QColor(255, 255, 255, 220)));
        customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignRight);
        
        // Grid
        customPlot->xAxis->grid()->setPen(QPen(QColor(220, 220, 220), 1, Qt::DotLine));
        customPlot->yAxis->grid()->setPen(QPen(QColor(220, 220, 220), 1, Qt::DotLine));
        customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(240, 240, 240), 1, Qt::DotLine));
        customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(240, 240, 240), 1, Qt::DotLine));
        customPlot->xAxis->grid()->setSubGridVisible(true);
        customPlot->yAxis->grid()->setSubGridVisible(true);
        
        // Styling
        customPlot->setBackground(QBrush(QColor(250, 250, 250)));
        customPlot->xAxis->setBasePen(QPen(Qt::black, 1));
        customPlot->yAxis->setBasePen(QPen(Qt::black, 1));
        customPlot->xAxis->setTickPen(QPen(Qt::black, 1));
        customPlot->yAxis->setTickPen(QPen(Qt::black, 1));
        customPlot->xAxis->setSubTickPen(QPen(Qt::black, 1));
        customPlot->yAxis->setSubTickPen(QPen(Qt::black, 1));
        customPlot->xAxis->setTickLabelColor(Qt::black);
        customPlot->yAxis->setTickLabelColor(Qt::black);
        customPlot->xAxis->setLabelColor(Qt::black);
        customPlot->yAxis->setLabelColor(Qt::black);
        
        // Antialiasing
        customPlot->setAntialiasedElements(QCP::aeAll);
        
        // Interactions
        customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        
        mainLayout->addWidget(customPlot);

        // === Process Table ===
        QLabel* procLabel = new QLabel("Laufende Prozesse (Nach Speicherauslastung)");
        procLabel->setStyleSheet("font-size: 14px; font-weight: bold; margin-top: 10px;");
        mainLayout->addWidget(procLabel);

        processTable = new QTableWidget();
        processTable->setColumnCount(3);
        processTable->setHorizontalHeaderLabels({"Prozessname", "PID", "Speicher (MB)"});
        processTable->horizontalHeader()->setStretchLastSection(false);
        processTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        processTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        processTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        processTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        processTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        processTable->setAlternatingRowColors(true);
        processTable->setStyleSheet(
            "QTableWidget { border: 1px solid #ddd; }"
            "QHeaderView::section { background-color: #2196F3; color: white; "
            "padding: 5px; font-weight: bold; }"
        );
        mainLayout->addWidget(processTable);

        // === Buttons ===
        QHBoxLayout* btnLayout = new QHBoxLayout();
        
        QPushButton* refreshBtn = new QPushButton("🔄 Prozesse aktualisieren");
        QPushButton* clearBtn = new QPushButton("🗑️ Graph zurücksetzen");
        
        QString btnStyle = 
            "QPushButton {"
            "  padding: 10px 20px; font-size: 13px; font-weight: bold;"
            "  background: #2196F3; color: white; border: none; border-radius: 5px;"
            "}"
            "QPushButton:hover { background: #1976D2; }"
            "QPushButton:pressed { background: #0D47A1; }";
        
        refreshBtn->setStyleSheet(btnStyle);
        clearBtn->setStyleSheet(btnStyle);
        
        connect(refreshBtn, &QPushButton::clicked, this, &MonitorWindow::updateProcessList);
        connect(clearBtn, &QPushButton::clicked, this, &MonitorWindow::clearChart);
        
        btnLayout->addWidget(refreshBtn);
        btnLayout->addWidget(clearBtn);
        btnLayout->addStretch();
        mainLayout->addLayout(btnLayout);

        // === Status Bar ===
        statusLabel = new QLabel("SystemMonitor läuft... | Update-Intervall: 1 Sekunde");
        statusLabel->setStyleSheet("color: #666; font-size: 11px;");
        mainLayout->addWidget(statusLabel);

        // === Timer für Updates ===
        updateTimer = new QTimer(this);
        connect(updateTimer, &QTimer::timeout, this, &MonitorWindow::updateData);
        updateTimer->start(1000); // 1 Sekunde

        // Initiales Update
        updateProcessList();
    }

private slots:
    void updateData() {
        // System-Daten holen
        double cpu = SystemInfo::getCpuUsage();
        auto [usedMem, totalMem] = SystemInfo::getMemoryUsage();
        double memPercent = (usedMem / totalMem) * 100.0;

        // Labels aktualisieren
        cpuLabel->setText(QString("CPU: %1%").arg(cpu, 0, 'f', 1));
        memLabel->setText(QString("RAM: %1 MB / %2 MB (%3%)")
            .arg(usedMem, 0, 'f', 0)
            .arg(totalMem, 0, 'f', 0)
            .arg(memPercent, 0, 'f', 1));

        // Daten hinzufügen
        timeData.append(currentTime);
        cpuData.append(cpu);
        memData.append(memPercent);
        currentTime += 1.0;

        // Nur letzte maxDataPoints behalten
        if (timeData.size() > maxDataPoints) {
            timeData.removeFirst();
            cpuData.removeFirst();
            memData.removeFirst();
        }

        // Graphen aktualisieren
        customPlot->graph(0)->setData(timeData, cpuData);
        customPlot->graph(1)->setData(timeData, memData);
        
        // X-Achse anpassen (Rolling Window)
        if (timeData.size() >= maxDataPoints) {
            customPlot->xAxis->setRange(timeData.first(), timeData.last());
        } else {
            customPlot->xAxis->setRange(0, maxDataPoints);
        }
        
        // Neu zeichnen
        customPlot->replot();
    }

    void updateProcessList() {
        auto processes = SystemInfo::getProcessList();
        
        // Nach Speicher sortieren (absteigend)
        std::sort(processes.begin(), processes.end(), 
            [](const auto& a, const auto& b) { return a.memoryMB > b.memoryMB; });

        // Top 50 anzeigen
        int rowCount = (int)processes.size();
        processTable->setRowCount(rowCount);
        
        for (int i = 0; i < rowCount; ++i) {
            auto* nameItem = new QTableWidgetItem(QString::fromStdString(processes[i].name));
            auto* pidItem = new QTableWidgetItem(QString::number(processes[i].pid));
            auto* memItem = new QTableWidgetItem(QString::number(processes[i].memoryMB, 'f', 2));
            
            // Zahlen rechts ausrichten
            pidItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            memItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            
            processTable->setItem(i, 0, nameItem);
            processTable->setItem(i, 1, pidItem);
            processTable->setItem(i, 2, memItem);
        }
        
        statusLabel->setText(QString("SystemMonitor läuft... | %1 Prozesse erkannt")
            .arg(processes.size()));
    }

    void clearChart() {
        timeData.clear();
        cpuData.clear();
        memData.clear();
        currentTime = 0.0;
        
        customPlot->graph(0)->data()->clear();
        customPlot->graph(1)->data()->clear();
        customPlot->xAxis->setRange(0, maxDataPoints);
        customPlot->replot();
        
        statusLabel->setText("Graph zurückgesetzt!");
    }
};

// ===== Main Function =====
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Windows-Stil setzen
    app.setStyle("Fusion");
    
    MonitorWindow window;
    window.show();
    
    return app.exec();
}

#include "main.moc"
