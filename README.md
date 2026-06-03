# Industrial Sensor Monitoring System

A real-time industrial sensor monitoring desktop application built with **C++17 and Qt 5/6**.  
Demonstrates core Qt competencies: signal-slot communication, multithreading with QThread,  
MVC architecture via QAbstractTableModel, and live data visualization using Qt Charts.

---

## Screenshots

> *Run the application to see the live dashboard, real-time charts, and alert panel.*

---

## Features

| Feature | Implementation |
|---|---|
| Real-time sensor data | `QTimer` + `QThread` worker pattern |
| Live data table | `QAbstractTableModel` (MVC) + `QTableView` |
| Live line charts | `Qt Charts` – `QLineSeries` / `QDateTimeAxis` |
| Alert management | Threshold-based `AlertManager` with severity levels |
| CSV data logging | Thread-safe `DataLogger` with `QMutex` |
| Dark UI theme | `Fusion` style + custom `QPalette` |
| Configurable update rate | Adjustable via toolbar `QSpinBox` |

**Sensors simulated:**
- Boiler Temperature & Coolant Temperature (°C)
- Main Pressure & Outlet Pressure (bar)
- Ambient Humidity (%)
- Pump Vibration (mm/s)
- Water Flow Rate (L/min)
- Supply Voltage (V)

---

## Architecture

```
┌──────────────────────────────────────────────────────────┐
│                      MainWindow                          │
│  (QMainWindow – orchestrates all components)             │
│                                                          │
│   ┌──────────────────┐      ┌────────────────────────┐  │
│   │  SensorThread    │      │    SensorDashboard     │  │
│   │  (QThread)       │      │    (QWidget)           │  │
│   │                  │      │                        │  │
│   │  ┌────────────┐  │      │  ┌──────────────────┐  │  │
│   │  │SensorWorker│──┼──────┼─▶│  SensorModel     │  │  │
│   │  │(QObject)   │  │signal│  │(QAbstractTable   │  │  │
│   │  │            │  │/slot │  │ Model) ──▶ View  │  │  │
│   │  └────────────┘  │      │  └──────────────────┘  │  │
│   └──────────────────┘      │  ┌──────────────────┐  │  │
│                              │  │SensorChartWidget │  │  │
│   ┌──────────────────┐      │  │(Qt Charts)       │  │  │
│   │   DataLogger     │      │  └──────────────────┘  │  │
│   │   (CSV logging)  │      │  ┌──────────────────┐  │  │
│   └──────────────────┘      │  │  AlertManager    │  │  │
│                              │  └──────────────────┘  │  │
│                              └────────────────────────┘  │
└──────────────────────────────────────────────────────────┘
```

### Key Design Patterns

**1. Worker-Object Threading (not QThread subclass)**
```cpp
// Worker lives in a separate thread; signals/slots cross thread boundary safely
m_worker->moveToThread(m_thread);
connect(m_thread, &QThread::started, m_worker, &SensorWorker::start);
```

**2. MVC with QAbstractTableModel**
```cpp
// SensorModel inherits QAbstractTableModel
// QTableView binds to it automatically — no manual UI refresh needed
m_tableView->setModel(m_model);
// Updating data triggers dataChanged() signal → view re-renders
emit dataChanged(index(row, 0), index(row, ColCount - 1));
```

**3. Signal-Slot across threads**
```cpp
// Qt automatically queues cross-thread connections
connect(worker, &SensorWorker::sensorDataUpdated,   // emitted in worker thread
        m_dashboard, &SensorDashboard::onSensorDataUpdated);  // runs in UI thread
```

---

## Project Structure

```
IndustrialSensorMonitor/
├── include/
│   ├── sensordata.h          # Core data struct + enums
│   ├── sensormodel.h         # QAbstractTableModel (MVC Model)
│   ├── sensorworker.h        # QThread + worker (background processing)
│   ├── sensorchartwidget.h   # Qt Charts real-time widget
│   ├── sensordashboard.h     # Central dashboard widget
│   ├── alertmanager.h        # Threshold alert system
│   ├── datalogger.h          # Thread-safe CSV logger
│   └── mainwindow.h          # Top-level QMainWindow
├── src/
│   ├── main.cpp
│   ├── mainwindow.cpp
│   ├── sensorworker.cpp
│   ├── sensormodel.cpp
│   ├── sensordashboard.cpp
│   ├── sensorchartwidget.cpp
│   ├── alertmanager.cpp
│   └── datalogger.cpp
├── resources/
│   └── resources.qrc
├── IndustrialSensorMonitor.pro   # qmake build file
├── CMakeLists.txt                # CMake build file
└── README.md
```

---

## Build Instructions

### Prerequisites

- Qt 5.12+ or Qt 6.x (with `Qt Charts` module)
- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
- qmake **or** CMake 3.16+

### Using qmake (Qt Creator)

```bash
# Open IndustrialSensorMonitor.pro in Qt Creator and click Build
# Or from terminal:
qmake IndustrialSensorMonitor.pro
make -j$(nproc)
./IndustrialSensorMonitor
```

### Using CMake

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build . --parallel
./IndustrialSensorMonitor
```

> **Note:** On Linux you may need to install `libqt5charts5-dev`  
> On Windows, ensure Qt Charts is included in your Qt installation.

---

## Usage

1. **Start Monitoring** – Click `▶ Start` in the toolbar to begin receiving sensor data.
2. **View Table** – The left panel shows all sensors with colour-coded status (green/orange/red).
3. **Charts** – Click any row to filter the chart to that sensor, or see all sensors at once.
4. **Alerts** – The alert panel logs threshold violations. Click `Acknowledge All` to dismiss.
5. **Logging** – Click `⏺ Start Logging` to write all readings to a timestamped CSV in `logs/`.
6. **Interval** – Adjust the update interval (250 ms – 10 s) via the toolbar spinner.

---

## Technologies & Concepts Demonstrated

- **Qt Widgets** – `QMainWindow`, `QTableView`, `QSplitter`, `QGroupBox`, custom `QWidget`
- **Qt Charts** – `QChart`, `QLineSeries`, `QDateTimeAxis`, `QValueAxis`
- **QThread / Multithreading** – Worker-object pattern, `QMutex`, thread-safe signal/slot
- **MVC Pattern** – `QAbstractTableModel` as the data model for `QTableView`
- **Signal & Slot** – Cross-thread and same-thread connections throughout
- **C++17** – Lambda captures, structured bindings, `if constexpr`
- **Resource System** – `.qrc` resource file

---

## License

MIT License — free to use and modify for personal or commercial projects.
