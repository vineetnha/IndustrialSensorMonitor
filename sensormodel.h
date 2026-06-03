#pragma once

#include <QAbstractTableModel>
#include <QVector>
#include <QColor>
#include "sensordata.h"

/**
 * @brief MVC Model class for sensor data displayed in QTableView.
 *
 * Inherits QAbstractTableModel to provide a clean separation between
 * data and presentation layers, following the Model-View-Controller pattern.
 */
class SensorModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns {
        ColID = 0,
        ColName,
        ColType,
        ColValue,
        ColUnit,
        ColStatus,
        ColTimestamp,
        ColCount
    };

    explicit SensorModel(QObject *parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Data management
    void updateSensor(const SensorData &sensorData);
    void addSensor(const SensorData &sensorData);
    SensorData getSensor(int row) const;
    SensorData getSensorById(int sensorId) const;
    QVector<SensorData> getAllSensors() const;
    void clearAll();

signals:
    void sensorStatusChanged(int sensorId, SensorStatus oldStatus, SensorStatus newStatus);
    void criticalSensorDetected(const SensorData &sensor);

private:
    QVector<SensorData> m_sensors;

    QColor statusColor(SensorStatus status) const;
    int findSensorIndex(int sensorId) const;
};
