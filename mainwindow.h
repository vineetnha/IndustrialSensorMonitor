#pragma once

#include <QMainWindow>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QSpinBox>
#include <QTimer>
#include "sensordashboard.h"
#include "sensorworker.h"
#include "datalogger.h"

/**
 * @brief Top-level application window.
 *
 * Owns the SensorThread (QThread + SensorWorker), SensorDashboard,
 * and DataLogger. Connects all cross-component signals and slots,
 * manages the toolbar actions, and shows status bar info.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onStartMonitoring();
    void onStopMonitoring();
    void onToggleLogging();
    void onIntervalChanged(int value);
    void onWorkerStarted();
    void onWorkerStopped();
    void onStatusUpdate();
    void onAbout();

private:
    SensorDashboard *m_dashboard;
    SensorThread    *m_sensorThread;
    DataLogger      *m_logger;

    // Toolbar actions
    QAction  *m_actStart;
    QAction  *m_actStop;
    QAction  *m_actLog;
    QSpinBox *m_spnInterval;

    // Status bar labels
    QLabel *m_lblStatus;
    QLabel *m_lblRecordCount;
    QLabel *m_lblUptime;

    QTimer  *m_uptimeTimer;
    QDateTime m_startTime;
    bool m_monitoring;
    bool m_logging;

    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void connectSignals();
};
