#include "mainwindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QCloseEvent>
#include <QLabel>
#include <QSpinBox>
#include <QDateTime>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_dashboard(new SensorDashboard(this))
    , m_sensorThread(new SensorThread(this))
    , m_logger(new DataLogger(this))
    , m_uptimeTimer(new QTimer(this))
    , m_monitoring(false)
    , m_logging(false)
{
    setWindowTitle("Industrial Sensor Monitoring System");
    setMinimumSize(1100, 650);
    setCentralWidget(m_dashboard);

    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    connectSignals();

    connect(m_uptimeTimer, &QTimer::timeout, this, &MainWindow::onStatusUpdate);
    m_uptimeTimer->start(1000);
}

MainWindow::~MainWindow()
{
    m_sensorThread->stopWorker();
    m_logger->stopLogging();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_sensorThread->stopWorker();
    m_logger->stopLogging();
    event->accept();
}

void MainWindow::onStartMonitoring()
{
    if (m_monitoring) return;
    m_monitoring = true;
    m_startTime = QDateTime::currentDateTime();

    m_actStart->setEnabled(false);
    m_actStop->setEnabled(true);
    m_lblStatus->setText("● Monitoring");
    m_lblStatus->setStyleSheet("color: #2ecc71; font-weight: bold;");

    m_sensorThread->startWorker();
}

void MainWindow::onStopMonitoring()
{
    if (!m_monitoring) return;
    m_monitoring = false;

    m_actStart->setEnabled(true);
    m_actStop->setEnabled(false);
    m_lblStatus->setText("■ Stopped");
    m_lblStatus->setStyleSheet("color: #e74c3c; font-weight: bold;");

    m_sensorThread->stopWorker();
}

void MainWindow::onToggleLogging()
{
    if (!m_logging) {
        if (m_logger->startLogging()) {
            m_logging = true;
            m_actLog->setText("Stop Logging");
            m_actLog->setToolTip("Stop CSV logging");
        }
    } else {
        m_logger->stopLogging();
        m_logging = false;
        m_actLog->setText("Start Logging");
        m_actLog->setToolTip("Log sensor data to CSV");
    }
}

void MainWindow::onIntervalChanged(int value)
{
    QMetaObject::invokeMethod(m_sensorThread->worker(), "setUpdateInterval",
                              Qt::QueuedConnection, Q_ARG(int, value));
}

void MainWindow::onWorkerStarted()
{
    qDebug() << "[MainWindow] Worker started.";
}

void MainWindow::onWorkerStopped()
{
    qDebug() << "[MainWindow] Worker stopped.";
}

void MainWindow::onStatusUpdate()
{
    if (!m_monitoring) return;

    int secs = m_startTime.secsTo(QDateTime::currentDateTime());
    int h = secs / 3600, m = (secs % 3600) / 60, s = secs % 60;
    m_lblUptime->setText(QString("Uptime: %1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0')));

    if (m_logging)
        m_lblRecordCount->setText(QString("Records: %1").arg(m_logger->recordsWritten()));
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About",
        "<h3>Industrial Sensor Monitoring System</h3>"
        "<p>Real-time sensor monitoring application built with <b>C++ and Qt</b>.</p>"
        "<ul>"
        "<li>Signal-Slot communication for real-time updates</li>"
        "<li>QThread-based background data processing</li>"
        "<li>MVC architecture (QAbstractTableModel)</li>"
        "<li>Qt Charts for live data visualization</li>"
        "<li>CSV data logging</li>"
        "</ul>"
        "<p><i>Built as a portfolio project demonstrating Qt/C++ expertise.</i></p>"
    );
}

void MainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Start Monitoring", this, &MainWindow::onStartMonitoring, QKeySequence("Ctrl+R"));
    fileMenu->addAction("S&top Monitoring",  this, &MainWindow::onStopMonitoring,  QKeySequence("Ctrl+T"));
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit", this, &QWidget::close, QKeySequence::Quit);

    auto *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About", this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar()
{
    auto *toolbar = addToolBar("Main Toolbar");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));

    m_actStart = toolbar->addAction("▶ Start");
    m_actStart->setToolTip("Start monitoring sensors");
    connect(m_actStart, &QAction::triggered, this, &MainWindow::onStartMonitoring);

    m_actStop = toolbar->addAction("■ Stop");
    m_actStop->setToolTip("Stop monitoring");
    m_actStop->setEnabled(false);
    connect(m_actStop, &QAction::triggered, this, &MainWindow::onStopMonitoring);

    toolbar->addSeparator();

    m_actLog = toolbar->addAction("⏺ Start Logging");
    m_actLog->setToolTip("Log sensor data to CSV");
    connect(m_actLog, &QAction::triggered, this, &MainWindow::onToggleLogging);

    toolbar->addSeparator();

    toolbar->addWidget(new QLabel("  Interval (ms): ", this));
    m_spnInterval = new QSpinBox(this);
    m_spnInterval->setRange(250, 10000);
    m_spnInterval->setValue(1000);
    m_spnInterval->setSingleStep(250);
    m_spnInterval->setToolTip("Update interval in milliseconds");
    connect(m_spnInterval, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onIntervalChanged);
    toolbar->addWidget(m_spnInterval);
}

void MainWindow::setupStatusBar()
{
    m_lblStatus = new QLabel("■ Stopped", this);
    m_lblStatus->setStyleSheet("color: #e74c3c; font-weight: bold;");

    m_lblUptime = new QLabel("Uptime: --:--:--", this);
    m_lblRecordCount = new QLabel("Records: 0", this);

    statusBar()->addWidget(m_lblStatus);
    statusBar()->addPermanentWidget(m_lblRecordCount);
    statusBar()->addPermanentWidget(m_lblUptime);
}

void MainWindow::connectSignals()
{
    SensorWorker *worker = m_sensorThread->worker();

    // Worker → Dashboard (cross-thread: Qt::QueuedConnection is automatic across threads)
    connect(worker, &SensorWorker::sensorDataUpdated,
            m_dashboard, &SensorDashboard::onSensorDataUpdated);

    // Worker → Logger
    connect(worker, &SensorWorker::sensorDataUpdated,
            m_logger, &DataLogger::logSensorData);

    // Worker lifecycle
    connect(m_sensorThread, &SensorThread::started, this, &MainWindow::onWorkerStarted);
    connect(m_sensorThread, &SensorThread::stopped, this, &MainWindow::onWorkerStopped);
}
