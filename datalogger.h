#pragma once

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include "sensordata.h"

/**
 * @brief Handles writing sensor readings to CSV log files.
 *
 * Thread-safe logging with mutex protection.
 * A new log file is created each session with a timestamped filename.
 */
class DataLogger : public QObject
{
    Q_OBJECT

public:
    explicit DataLogger(QObject *parent = nullptr);
    ~DataLogger();

    bool startLogging(const QString &directory = "logs");
    void stopLogging();
    bool isLogging() const { return m_logging; }
    QString currentLogFile() const { return m_logFilePath; }
    quint64 recordsWritten() const { return m_recordCount; }

public slots:
    void logSensorData(const SensorData &data);

signals:
    void loggingStarted(const QString &filePath);
    void loggingStopped(quint64 totalRecords);
    void logError(const QString &error);

private:
    QFile       *m_file;
    QTextStream *m_stream;
    QMutex       m_mutex;
    QString      m_logFilePath;
    bool         m_logging;
    quint64      m_recordCount;

    void writeHeader();
};
