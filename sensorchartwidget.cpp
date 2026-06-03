#include "sensorchartwidget.h"
#include <QVBoxLayout>
#include <QDateTime>

SensorChartWidget::SensorChartWidget(QWidget *parent)
    : QWidget(parent)
    , m_maxDataPoints(60)
    , m_timeWindowSeconds(60)
    , m_filterSensorId(-1)
{
    setupChart();
}

void SensorChartWidget::setSensorFilter(int sensorId)
{
    m_filterSensorId = sensorId;
    clearChart();
}

void SensorChartWidget::setMaxDataPoints(int count)
{
    m_maxDataPoints = count;
}

void SensorChartWidget::setTimeWindowSeconds(int seconds)
{
    m_timeWindowSeconds = seconds;
}

void SensorChartWidget::clearChart()
{
    for (auto *series : m_seriesMap.values())
        m_chart->removeSeries(series);

    qDeleteAll(m_seriesMap);
    m_seriesMap.clear();
    m_dataBuffer.clear();
}

void SensorChartWidget::onSensorDataUpdated(const SensorData &data)
{
    if (m_filterSensorId != -1 && data.sensorId != m_filterSensorId)
        return;

    QLineSeries *series = getOrCreateSeries(data);
    auto &buffer = m_dataBuffer[data.sensorId];

    buffer.append({data.timestamp, data.value});
    if (buffer.size() > m_maxDataPoints)
        buffer.removeFirst();

    // Rebuild series points from buffer
    QVector<QPointF> points;
    points.reserve(buffer.size());
    for (const auto &p : qAsConst(buffer))
        points.append(QPointF(p.first.toMSecsSinceEpoch(), p.second));

    series->replace(points);
    updateAxes();
}

// ── Private ─────────────────────────────────────────────────────────────────

void SensorChartWidget::setupChart()
{
    m_chart = new QChart();
    m_chart->setTitle("Live Sensor Data");
    m_chart->setTheme(QChart::ChartThemeDark);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    m_axisX = new QDateTimeAxis(this);
    m_axisX->setFormat("hh:mm:ss");
    m_axisX->setTitleText("Time");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    m_axisY = new QValueAxis(this);
    m_axisY->setTitleText("Value");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chartView);
}

QLineSeries* SensorChartWidget::getOrCreateSeries(const SensorData &data)
{
    if (m_seriesMap.contains(data.sensorId))
        return m_seriesMap[data.sensorId];

    auto *series = new QLineSeries(this);
    series->setName(data.sensorName);
    series->setColor(seriesColor(data.sensorId));

    QPen pen = series->pen();
    pen.setWidth(2);
    series->setPen(pen);

    m_chart->addSeries(series);
    series->attachAxis(m_axisX);
    series->attachAxis(m_axisY);

    m_seriesMap[data.sensorId] = series;
    return series;
}

void SensorChartWidget::updateAxes()
{
    QDateTime now = QDateTime::currentDateTime();
    m_axisX->setRange(now.addSecs(-m_timeWindowSeconds), now);

    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();

    for (const auto &buf : qAsConst(m_dataBuffer)) {
        for (const auto &p : buf) {
            yMin = qMin(yMin, p.second);
            yMax = qMax(yMax, p.second);
        }
    }

    if (yMin < yMax) {
        double margin = (yMax - yMin) * 0.1;
        m_axisY->setRange(yMin - margin, yMax + margin);
    }
}

QColor SensorChartWidget::seriesColor(int sensorId) const
{
    static const QList<QColor> palette = {
        QColor("#3498db"), QColor("#e74c3c"), QColor("#2ecc71"),
        QColor("#f39c12"), QColor("#9b59b6"), QColor("#1abc9c"),
        QColor("#e67e22"), QColor("#e91e63")
    };
    return palette.at(sensorId % palette.size());
}
