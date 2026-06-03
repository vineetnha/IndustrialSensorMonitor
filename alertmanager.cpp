#include "alertmanager.h"

AlertManager::AlertManager(QObject *parent)
    : QObject(parent)
{}

QVector<AlertEntry> AlertManager::activeAlerts() const
{
    return m_activeAlerts;
}

QVector<AlertEntry> AlertManager::alertHistory() const
{
    return m_history;
}

int AlertManager::unacknowledgedCount() const
{
    int count = 0;
    for (const AlertEntry &a : m_activeAlerts)
        if (!a.acknowledged) ++count;
    return count;
}

void AlertManager::acknowledgeAlert(int sensorId)
{
    for (AlertEntry &a : m_activeAlerts) {
        if (a.sensorId == sensorId)
            a.acknowledged = true;
    }
    emit alertCountChanged(unacknowledgedCount());
}

void AlertManager::acknowledgeAll()
{
    for (AlertEntry &a : m_activeAlerts)
        a.acknowledged = true;
    emit alertCountChanged(0);
}

void AlertManager::clearHistory()
{
    m_history.clear();
}

void AlertManager::onSensorDataUpdated(const SensorData &data)
{
    bool currentlyAlerting = hasActiveAlert(data.sensorId);

    if (data.status == SensorStatus::Normal || data.status == SensorStatus::Offline) {
        if (currentlyAlerting) {
            // Remove resolved alert
            m_activeAlerts.erase(
                std::remove_if(m_activeAlerts.begin(), m_activeAlerts.end(),
                    [&](const AlertEntry &a) { return a.sensorId == data.sensorId; }),
                m_activeAlerts.end()
            );
            emit alertResolved(data.sensorId);
            emit alertCountChanged(unacknowledgedCount());
        }
        return;
    }

    // Warning or Critical
    if (!currentlyAlerting) {
        AlertEntry entry;
        entry.sensorId    = data.sensorId;
        entry.sensorName  = data.sensorName;
        entry.severity    = data.status;
        entry.message     = buildAlertMessage(data);
        entry.timestamp   = data.timestamp;
        entry.acknowledged = false;

        m_activeAlerts.append(entry);
        m_history.append(entry);

        // Keep history to 200 entries max
        if (m_history.size() > 200)
            m_history.removeFirst();

        emit alertTriggered(entry);
        emit alertCountChanged(unacknowledgedCount());
    }
}

bool AlertManager::hasActiveAlert(int sensorId) const
{
    for (const AlertEntry &a : m_activeAlerts)
        if (a.sensorId == sensorId) return true;
    return false;
}

QString AlertManager::buildAlertMessage(const SensorData &data) const
{
    QString level = (data.status == SensorStatus::Critical) ? "CRITICAL" : "WARNING";
    return QString("[%1] %2: %.2f %3 (range: %.1f–%.1f)")
        .arg(level)
        .arg(data.sensorName)
        .arg(data.value)
        .arg(data.unit)
        .arg(data.minThreshold)
        .arg(data.maxThreshold);
}
