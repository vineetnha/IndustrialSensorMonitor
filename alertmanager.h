#pragma once

#include <QObject>
#include <QVector>
#include <QDateTime>
#include "sensordata.h"

/**
 * @brief Represents a single alert event triggered by a sensor threshold violation
 */
struct AlertEntry {
    int          sensorId;
    QString      sensorName;
    SensorStatus severity;
    QString      message;
    QDateTime    timestamp;
    bool         acknowledged;
};

/**
 * @brief Manages alert generation, storage, and acknowledgement.
 *
 * Receives sensor data updates, evaluates threshold conditions,
 * and emits signals when alert states change. Maintains an in-memory
 * log of all alerts for display in the dashboard.
 */
class AlertManager : public QObject
{
    Q_OBJECT

public:
    explicit AlertManager(QObject *parent = nullptr);

    QVector<AlertEntry> activeAlerts() const;
    QVector<AlertEntry> alertHistory() const;
    int unacknowledgedCount() const;
    void acknowledgeAlert(int sensorId);
    void acknowledgeAll();
    void clearHistory();

public slots:
    void onSensorDataUpdated(const SensorData &data);

signals:
    void alertTriggered(const AlertEntry &alert);
    void alertResolved(int sensorId);
    void alertCountChanged(int count);

private:
    QVector<AlertEntry> m_activeAlerts;
    QVector<AlertEntry> m_history;

    bool hasActiveAlert(int sensorId) const;
    QString buildAlertMessage(const SensorData &data) const;
};
