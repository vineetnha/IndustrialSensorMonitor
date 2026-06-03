#include "datalogger.h"
#include <QDir>
#include <QMutexLocker>
#include <QDebug>

DataLogger::DataLogger(QObject *parent)
    : QObject(parent)
    , m_file(nullptr)
    , m_stream(nullptr)
    , m_logging(false)
    , m_recordCount(0)
{}

DataLogger::~DataLogger()
{
    stopLogging();
}

bool DataLogger::startLogging(const QString &directory)
{
    QMutexLocker locker(&m_mutex);
    if (m_logging) return true;

    QDir dir;
    if (!dir.exists(directory))
        dir.mkpath(directory);

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    m_logFilePath = directory + "/sensor_log_" + timestamp + ".csv";

    m_file = new QFile(m_logFilePath, this);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit logError("Cannot open log file: " + m_logFilePath);
        delete m_file;
        m_file = nullptr;
        return false;
    }

    m_stream = new QTextStream(m_file);
    m_recordCount = 0;
    m_logging = true;

    writeHeader();
    emit loggingStarted(m_logFilePath);
    qDebug() << "[DataLogger] Logging started:" << m_logFilePath;
    return true;
}

void DataLogger::stopLogging()
{
    QMutexLocker locker(&m_mutex);
    if (!m_logging) return;

    m_logging = false;

    if (m_stream) {
        m_stream->flush();
        delete m_stream;
        m_stream = nullptr;
    }
    if (m_file) {
        m_file->close();
        delete m_file;
        m_file = nullptr;
    }

    emit loggingStopped(m_recordCount);
    qDebug() << "[DataLogger] Stopped. Records written:" << m_recordCount;
}

void DataLogger::logSensorData(const SensorData &data)
{
    QMutexLocker locker(&m_mutex);
    if (!m_logging || !m_stream) return;

    *m_stream << data.timestamp.toString(Qt::ISODate) << ","
              << data.sensorId << ","
              << data.sensorName << ","
              << SensorData::typeToString(data.type) << ","
              << QString::number(data.value, 'f', 4) << ","
              << data.unit << ","
              << SensorData::statusToString(data.status) << "\n";

    ++m_recordCount;

    // Flush every 50 records to avoid data loss on crash
    if (m_recordCount % 50 == 0)
        m_stream->flush();
}

void DataLogger::writeHeader()
{
    *m_stream << "Timestamp,SensorID,SensorName,Type,Value,Unit,Status\n";
}
