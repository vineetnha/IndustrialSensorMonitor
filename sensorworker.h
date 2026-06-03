#pragma once

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <QMutex>
#include <QRandomGenerator>
#include "sensordata.h"

/**
 * @brief Worker object that runs in a separate QThread to simulate
 *        and process real-time sensor data without blocking the UI thread.
 *
 * Uses the Worker-Object pattern (not subclassing QThread directly)
 * for better separation of concerns and thread safety.
 */
class SensorWorker : public QObject
{
    Q_OBJECT

public:
    explicit SensorWorker(QObject *parent = nullptr);
    ~SensorWorker();

public slots:
    void start();
    void stop();
    void setUpdateInterval(int ms);
    void setSensorEnabled(int sensorId, bool enabled);

signals:
    void sensorDataUpdated(const SensorData &data);
    void allSensorsUpdated(const QVector<SensorData> &allData);
    void workerStarted();
    void workerStopped();
    void errorOccurred(const QString &error);

private slots:
    void fetchSensorData();

private:
    QTimer              *m_timer;
    QVector<SensorData>  m_sensorConfig;
    QMutex               m_mutex;
    bool                 m_running;
    int                  m_updateIntervalMs;

    void initializeSensors();
    SensorData simulateSensorReading(const SensorData &config);
    double generateRealisticValue(const SensorData &config);
};

/**
 * @brief Thread manager that owns and manages the SensorWorker lifecycle.
 *
 * Demonstrates the recommended Qt pattern: create a QThread, move the
 * worker to it, and connect signals/slots across thread boundaries.
 */
class SensorThread : public QObject
{
    Q_OBJECT

public:
    explicit SensorThread(QObject *parent = nullptr);
    ~SensorThread();

    void startWorker();
    void stopWorker();
    SensorWorker* worker() const { return m_worker; }

signals:
    void started();
    void stopped();

private:
    QThread     *m_thread;
    SensorWorker *m_worker;
};
