#pragma once

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QMap>
#include <QVector>
#include <QPair>
#include "sensordata.h"

QT_CHARTS_USE_NAMESPACE

/**
 * @brief Widget that renders real-time line charts for sensor data.
 *
 * Maintains a rolling window of historical readings per sensor,
 * and updates the chart dynamically as new data arrives via signals.
 */
class SensorChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SensorChartWidget(QWidget *parent = nullptr);

    void setSensorFilter(int sensorId);   // -1 = show all
    void setMaxDataPoints(int count);
    void setTimeWindowSeconds(int seconds);
    void clearChart();

public slots:
    void onSensorDataUpdated(const SensorData &data);

private:
    QChartView  *m_chartView;
    QChart      *m_chart;
    QDateTimeAxis *m_axisX;
    QValueAxis    *m_axisY;

    // sensorId -> series
    QMap<int, QLineSeries*> m_seriesMap;

    // sensorId -> ring buffer of (timestamp, value)
    QMap<int, QVector<QPair<QDateTime, double>>> m_dataBuffer;

    int m_maxDataPoints;
    int m_timeWindowSeconds;
    int m_filterSensorId;   // -1 = all

    void setupChart();
    QLineSeries* getOrCreateSeries(const SensorData &data);
    void updateAxes();
    QColor seriesColor(int sensorId) const;
};
