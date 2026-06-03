#include "sensormodel.h"
#include <QBrush>
#include <QFont>

SensorModel::SensorModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

int SensorModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_sensors.size();
}

int SensorModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return ColCount;
}

QVariant SensorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_sensors.size())
        return QVariant();

    const SensorData &s = m_sensors.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case ColID:        return s.sensorId;
            case ColName:      return s.sensorName;
            case ColType:      return SensorData::typeToString(s.type);
            case ColValue:     return QString::number(s.value, 'f', 2);
            case ColUnit:      return s.unit;
            case ColStatus:    return SensorData::statusToString(s.status);
            case ColTimestamp: return s.timestamp.toString("hh:mm:ss");
            default:           return QVariant();
        }
    }

    if (role == Qt::BackgroundRole) {
        return QBrush(statusColor(s.status));
    }

    if (role == Qt::ForegroundRole) {
        if (s.status == SensorStatus::Critical || s.status == SensorStatus::Warning)
            return QBrush(Qt::white);
        return QVariant();
    }

    if (role == Qt::FontRole && s.status == SensorStatus::Critical) {
        QFont f;
        f.setBold(true);
        return f;
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == ColValue || index.column() == ColID)
            return Qt::AlignCenter;
    }

    return QVariant();
}

QVariant SensorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
        case ColID:        return "ID";
        case ColName:      return "Sensor Name";
        case ColType:      return "Type";
        case ColValue:     return "Value";
        case ColUnit:      return "Unit";
        case ColStatus:    return "Status";
        case ColTimestamp: return "Last Update";
        default:           return QVariant();
    }
}

void SensorModel::updateSensor(const SensorData &sensorData)
{
    int idx = findSensorIndex(sensorData.sensorId);
    if (idx == -1) {
        addSensor(sensorData);
        return;
    }

    SensorStatus oldStatus = m_sensors[idx].status;
    m_sensors[idx] = sensorData;

    emit dataChanged(index(idx, 0), index(idx, ColCount - 1));

    if (oldStatus != sensorData.status) {
        emit sensorStatusChanged(sensorData.sensorId, oldStatus, sensorData.status);
        if (sensorData.status == SensorStatus::Critical)
            emit criticalSensorDetected(sensorData);
    }
}

void SensorModel::addSensor(const SensorData &sensorData)
{
    beginInsertRows(QModelIndex(), m_sensors.size(), m_sensors.size());
    m_sensors.append(sensorData);
    endInsertRows();
}

SensorData SensorModel::getSensor(int row) const
{
    Q_ASSERT(row >= 0 && row < m_sensors.size());
    return m_sensors.at(row);
}

SensorData SensorModel::getSensorById(int sensorId) const
{
    int idx = findSensorIndex(sensorId);
    if (idx != -1) return m_sensors.at(idx);
    return SensorData{};
}

QVector<SensorData> SensorModel::getAllSensors() const
{
    return m_sensors;
}

void SensorModel::clearAll()
{
    beginResetModel();
    m_sensors.clear();
    endResetModel();
}

// ── Private ────────────────────────────────────────────────────────────────

QColor SensorModel::statusColor(SensorStatus status) const
{
    switch (status) {
        case SensorStatus::Normal:   return QColor("#2ecc71");   // green
        case SensorStatus::Warning:  return QColor("#f39c12");   // orange
        case SensorStatus::Critical: return QColor("#e74c3c");   // red
        case SensorStatus::Offline:  return QColor("#95a5a6");   // grey
        default:                     return QColor(Qt::white);
    }
}

int SensorModel::findSensorIndex(int sensorId) const
{
    for (int i = 0; i < m_sensors.size(); ++i) {
        if (m_sensors[i].sensorId == sensorId)
            return i;
    }
    return -1;
}
