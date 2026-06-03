#include "sensordashboard.h"
#include <QHeaderView>
#include <QListWidget>
#include <QPushButton>
#include <QSplitter>
#include <QFont>

SensorDashboard::SensorDashboard(QWidget *parent)
    : QWidget(parent)
    , m_model(new SensorModel(this))
    , m_alertManager(new AlertManager(this))
{
    setupUi();

    connect(m_alertManager, &AlertManager::alertTriggered,
            this, &SensorDashboard::onAlertTriggered);
    connect(m_alertManager, &AlertManager::alertCountChanged,
            this, &SensorDashboard::onAlertCountChanged);
    connect(m_model, &SensorModel::criticalSensorDetected,
            m_alertManager, &AlertManager::onSensorDataUpdated);
}

void SensorDashboard::onSensorDataUpdated(const SensorData &data)
{
    m_model->updateSensor(data);
    m_chart->onSensorDataUpdated(data);
    m_alertManager->onSensorDataUpdated(data);
    updateStatusSummary();
}

void SensorDashboard::onAlertTriggered(const AlertEntry &alert)
{
    QString icon = (alert.severity == SensorStatus::Critical) ? "🔴" : "🟡";
    QString text = QString("%1 [%2] %3")
        .arg(icon)
        .arg(alert.timestamp.toString("hh:mm:ss"))
        .arg(alert.message);

    m_alertList->insertItem(0, text);

    // Keep list manageable
    while (m_alertList->count() > 50)
        delete m_alertList->takeItem(m_alertList->count() - 1);
}

void SensorDashboard::onAlertCountChanged(int count)
{
    m_lblAlertBadge->setText(count > 0 ? QString("⚠ %1 Alert(s)").arg(count) : "✔ No Alerts");
    m_lblAlertBadge->setStyleSheet(count > 0
        ? "color: white; background: #e74c3c; padding: 4px 8px; border-radius: 4px;"
        : "color: white; background: #2ecc71; padding: 4px 8px; border-radius: 4px;");
}

void SensorDashboard::onTableSelectionChanged(const QModelIndex &current, const QModelIndex &)
{
    if (!current.isValid()) return;
    SensorData s = m_model->getSensor(current.row());
    m_chart->setSensorFilter(s.sensorId);
}

void SensorDashboard::onAcknowledgeAllClicked()
{
    m_alertManager->acknowledgeAll();
}

// ── Private ──────────────────────────────────────────────────────────────────

void SensorDashboard::setupUi()
{
    // ── Status summary bar ──────────────────────────────────────────────
    auto *summaryBar = new QWidget(this);
    summaryBar->setObjectName("summaryBar");
    summaryBar->setStyleSheet("background: #2c3e50; border-radius: 6px;");
    auto *summaryLayout = new QHBoxLayout(summaryBar);
    summaryLayout->setContentsMargins(12, 8, 12, 8);

    summaryLayout->addWidget(createStatusCard("Normal",   m_lblNormalCount,   "#2ecc71"));
    summaryLayout->addWidget(createStatusCard("Warning",  m_lblWarningCount,  "#f39c12"));
    summaryLayout->addWidget(createStatusCard("Critical", m_lblCriticalCount, "#e74c3c"));
    summaryLayout->addWidget(createStatusCard("Offline",  m_lblOfflineCount,  "#95a5a6"));
    summaryLayout->addStretch();

    m_lblAlertBadge = new QLabel("✔ No Alerts", this);
    m_lblAlertBadge->setStyleSheet(
        "color: white; background: #2ecc71; padding: 4px 8px; border-radius: 4px;");
    summaryLayout->addWidget(m_lblAlertBadge);

    // ── Table view ───────────────────────────────────────────────────────
    m_tableView = new QTableView(this);
    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_tableView->setSortingEnabled(true);
    styleTable();

    connect(m_tableView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &SensorDashboard::onTableSelectionChanged);

    // ── Chart ────────────────────────────────────────────────────────────
    m_chart = new SensorChartWidget(this);
    m_chart->setMinimumHeight(220);

    // ── Alert panel ──────────────────────────────────────────────────────
    m_alertList = new QListWidget(this);
    m_alertList->setMaximumHeight(150);
    m_alertList->setStyleSheet("font-size: 12px;");

    auto *ackBtn = new QPushButton("Acknowledge All", this);
    connect(ackBtn, &QPushButton::clicked, this, &SensorDashboard::onAcknowledgeAllClicked);

    auto *alertBox = new QGroupBox("Alert Log", this);
    auto *alertLayout = new QVBoxLayout(alertBox);
    alertLayout->addWidget(m_alertList);
    alertLayout->addWidget(ackBtn);

    // ── Right panel: chart + alerts ──────────────────────────────────────
    auto *rightPanel = new QWidget(this);
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addWidget(m_chart);
    rightLayout->addWidget(alertBox);

    // ── Splitter ─────────────────────────────────────────────────────────
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(m_tableView);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);

    // ── Root layout ───────────────────────────────────────────────────────
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);
    root->addWidget(summaryBar);
    root->addWidget(splitter, 1);
}

QWidget* SensorDashboard::createStatusCard(const QString &title, QLabel *&valueLabel,
                                            const QString &color)
{
    auto *card = new QFrame(this);
    card->setFrameShape(QFrame::StyledPanel);
    card->setStyleSheet(QString("background: %1; border-radius: 4px; padding: 4px;").arg(color));
    card->setFixedWidth(110);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(6, 4, 6, 4);

    auto *lblTitle = new QLabel(title, card);
    lblTitle->setStyleSheet("color: white; font-size: 11px;");
    lblTitle->setAlignment(Qt::AlignCenter);

    valueLabel = new QLabel("0", card);
    QFont f = valueLabel->font();
    f.setPointSize(18);
    f.setBold(true);
    valueLabel->setFont(f);
    valueLabel->setStyleSheet("color: white;");
    valueLabel->setAlignment(Qt::AlignCenter);

    cardLayout->addWidget(lblTitle);
    cardLayout->addWidget(valueLabel);
    return card;
}

void SensorDashboard::updateStatusSummary()
{
    int normal = 0, warning = 0, critical = 0, offline = 0;
    for (const SensorData &s : m_model->getAllSensors()) {
        switch (s.status) {
            case SensorStatus::Normal:   ++normal;   break;
            case SensorStatus::Warning:  ++warning;  break;
            case SensorStatus::Critical: ++critical; break;
            case SensorStatus::Offline:  ++offline;  break;
        }
    }
    m_lblNormalCount->setText(QString::number(normal));
    m_lblWarningCount->setText(QString::number(warning));
    m_lblCriticalCount->setText(QString::number(critical));
    m_lblOfflineCount->setText(QString::number(offline));
}

void SensorDashboard::styleTable()
{
    m_tableView->setStyleSheet(
        "QTableView {"
        "  gridline-color: #dfe6e9;"
        "  font-size: 13px;"
        "}"
        "QHeaderView::section {"
        "  background-color: #2c3e50;"
        "  color: white;"
        "  padding: 6px;"
        "  font-weight: bold;"
        "  border: 1px solid #34495e;"
        "}"
    );
}
