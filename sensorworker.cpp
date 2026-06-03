#include "sensorworker.h"
#include <QMutexLocker>
#include <QDebug>
#include <cmath>

// ── SensorWorker ────────────────────────────────────────────────────────────

SensorWorker::SensorWorker(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_running(false)
    , m_updateIntervalMs(1000)
{
    initializeSensors();
    connect(m_timer, &QTimer::timeout, this, &SensorWorker::fetchSensorData);
}

SensorWorker::~SensorWorker()
{
    stop();
}

void SensorWorker::start()
{
    QMutexLocker locker(&m_mutex);
    if (m_running) return;
    m_running = true;
    m_timer->start(m_updateIntervalMs);
    emit workerStarted();
    qDebug() << "[SensorWorker] Started in thread:" << QThread::currentThreadId();
}

void SensorWorker::stop()
{
    QMutexLocker locker(&m_mutex);
    if (!m_running) return;
    m_running = false;
    m_timer->stop();
    emit workerStopped();
    qDebug() << "[SensorWorker] Stopped.";
}

void SensorWorker::setUpdateInterval(int ms)
{
    QMutexLocker locker(&m_mutex);
    m_updateIntervalMs = ms;
    if (m_running) {
        m_timer->setInterval(ms);
    }
}

void SensorWorker::setSensorEnabled(int sensorId, bool enabled)
{
    QMutexLocker locker(&m_mutex);
    for (SensorData &s : m_sensorConfig) {
        if (s.sensorId == sensorId) {
            s.status = enabled ? SensorStatus::Normal : SensorStatus::Offline;
            break;
        }
    }
}

void SensorWorker::fetchSensorData()
{
    QVector<SensorData> batch;

    {
        QMutexLocker locker(&m_mutex);
        for (const SensorData &config : qAsConst(m_sensorConfig)) {
            batch.append(simulateSensorReading(config));
        }
    }

    for (const SensorData &d : qAsConst(batch)) {
        emit sensorDataUpdated(d);
    }
    emit allSensorsUpdated(batch);
}

void SensorWorker::initializeSensors()
{
    auto makeSensor = [](int id, const QString &name, SensorType type,
                         const QString &unit, double minT, double maxT) -> SensorData {
        SensorData s;
        s.sensorId     = id;
        s.sensorName   = name;
        s.type         = type;
        s.unit         = unit;
        s.value        = (minT + maxT) / 2.0;
        s.status       = SensorStatus::Normal;
        s.minThreshold = minT;
        s.maxThreshold = maxT;
        s.timestamp    = QDateTime::currentDateTime();
        return s;
    };

    m_sensorConfig = {
        makeSensor(1,  "Boiler Temp",       SensorType::Temperature, "°C",  60.0,  95.0),
        makeSensor(2,  "Coolant Temp",      SensorType::Temperature, "°C",  15.0,  40.0),
        makeSensor(3,  "Main Pressure",     SensorType::Pressure,    "bar",  1.0,   6.0),
        makeSensor(4,  "Outlet Pressure",   SensorType::Pressure,    "bar",  0.5,   4.0),
        makeSensor(5,  "Ambient Humidity",  SensorType::Humidity,    "%",   30.0,  70.0),
        makeSensor(6,  "Pump Vibration",    SensorType::Vibration,   "mm/s", 0.0,   4.5),
        makeSensor(7,  "Water Flow Rate",   SensorType::FlowRate,    "L/min",5.0,  50.0),
        makeSensor(8,  "Supply Voltage",    SensorType::Voltage,     "V",  220.0, 240.0),
    };
}

SensorData SensorWorker::simulateSensorReading(const SensorData &config)
{
    SensorData reading = config;
    reading.value     = generateRealisticValue(config);
    reading.status    = reading.evaluateStatus();
    reading.timestamp = QDateTime::currentDateTime();
    return reading;
}

double SensorWorker::generateRealisticValue(const SensorData &config)
{
    if (config.status == SensorStatus::Offline)
        return 0.0;

    double mid   = (config.minThreshold + config.maxThreshold) / 2.0;
    double range = (config.maxThreshold - config.minThreshold) / 2.0;

    // Simulate slow drift + random noise
    static QMap<int, double> driftMap;
    double &drift = driftMap[config.sensorId];

    // Drift towards mid slowly
    drift += QRandomGenerator::global()->bounded(-0.5, 0.5);
    drift  = qBound(-range * 0.3, drift, range * 0.3);

    double noise = QRandomGenerator::global()->bounded(-range * 0.05, range * 0.05);

    // Occasionally simulate a spike (5% chance)
    bool spike = QRandomGenerator::global()->bounded(100) < 5;
    double spikeVal = spike ? QRandomGenerator::global()->bounded(-range * 0.5, range * 0.5) : 0.0;

    return mid + drift + noise + spikeVal;
}

// ── SensorThread ─────────────────────────────────────────────────────────────

SensorThread::SensorThread(QObject *parent)
    : QObject(parent)
    , m_thread(new QThread(this))
    , m_worker(new SensorWorker())
{
    // Move worker off the main thread
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started,  m_worker, &SensorWorker::start);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);

    connect(m_worker, &SensorWorker::workerStarted, this, &SensorThread::started);
    connect(m_worker, &SensorWorker::workerStopped, this, &SensorThread::stopped);
}

SensorThread::~SensorThread()
{
    stopWorker();
}

void SensorThread::startWorker()
{
    if (!m_thread->isRunning())
        m_thread->start();
}

void SensorThread::stopWorker()
{
    if (m_thread->isRunning()) {
        QMetaObject::invokeMethod(m_worker, "stop", Qt::QueuedConnection);
        m_thread->quit();
        m_thread->wait(3000);
    }
}
