#pragma once

#include <QWidget>
#include <QTableView>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include "sensormodel.h"
#include "sensorchartwidget.h"
#include "alertmanager.h"

/**
 * @brief The central dashboard widget displaying sensor status panels,
 *        a live data table, and real-time charts.
 *
 * Composes the MVC table view (SensorModel + QTableView),
 * the SensorChartWidget, and summary status cards into one
 * unified dashboard layout.
 */
class SensorDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit SensorDashboard(QWidget *parent = nullptr);

    SensorModel       *sensorModel()  const { return m_model; }
    SensorChartWidget *chartWidget()  const { return m_chart; }
    AlertManager      *alertManager() const { return m_alertManager; }

public slots:
    void onSensorDataUpdated(const SensorData &data);
    void onAlertTriggered(const AlertEntry &alert);
    void onAlertCountChanged(int count);

private slots:
    void onTableSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void onAcknowledgeAllClicked();

private:
    // MVC components
    SensorModel  *m_model;
    QTableView   *m_tableView;

    // Sub-widgets
    SensorChartWidget *m_chart;
    AlertManager      *m_alertManager;

    // Status summary cards
    QLabel *m_lblNormalCount;
    QLabel *m_lblWarningCount;
    QLabel *m_lblCriticalCount;
    QLabel *m_lblOfflineCount;
    QLabel *m_lblAlertBadge;

    // Alert log panel
    QListWidget *m_alertList;

    void setupUi();
    QWidget* createStatusCard(const QString &title, QLabel *&valueLabel, const QString &color);
    void updateStatusSummary();
    void styleTable();
};
