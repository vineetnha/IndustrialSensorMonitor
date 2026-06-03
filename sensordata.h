#pragma once

#include <QString>
#include <QDateTime>

/**
 * @brief Enumeration of sensor types supported by the system
 */
enum class SensorType {
    Temperature,
    Pressure,
    Humidity,
    Vibration,
    FlowRate,
    Voltage
};

/**
 * @brief Enumeration representing the operational status of a sensor
 */
enum class SensorStatus {
    Normal,
    Warning,
    Critical,
    Offline
};

/**
 * @brief Core data structure representing a single sensor reading
 */
struct SensorData {
    int         sensorId;
    QString     sensorName;
    SensorType  type;
    double      value;
    QString     unit;
    SensorStatus status;
    QDateTime   timestamp;

    double      minThreshold;
    double      maxThreshold;

    /**
     * @brief Evaluates and returns the current status based on thresholds
     */
    SensorStatus evaluateStatus() const {
        if (value < minThreshold * 0.9 || value > maxThreshold * 1.1)
            return SensorStatus::Critical;
        if (value < minThreshold || value > maxThreshold)
            return SensorStatus::Warning;
        return SensorStatus::Normal;
    }

    /**
     * @brief Returns the sensor type as a human-readable string
     */
    static QString typeToString(SensorType t) {
        switch (t) {
            case SensorType::Temperature: return "Temperature";
            case SensorType::Pressure:    return "Pressure";
            case SensorType::Humidity:    return "Humidity";
            case SensorType::Vibration:   return "Vibration";
            case SensorType::FlowRate:    return "Flow Rate";
            case SensorType::Voltage:     return "Voltage";
            default:                      return "Unknown";
        }
    }

    /**
     * @brief Returns the sensor status as a human-readable string
     */
    static QString statusToString(SensorStatus s) {
        switch (s) {
            case SensorStatus::Normal:   return "Normal";
            case SensorStatus::Warning:  return "Warning";
            case SensorStatus::Critical: return "Critical";
            case SensorStatus::Offline:  return "Offline";
            default:                     return "Unknown";
        }
    }
};
