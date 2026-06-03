QT += core gui widgets charts

CONFIG += c++17

TARGET = IndustrialSensorMonitor
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/sensorworker.cpp \
    src/sensormodel.cpp \
    src/sensordashboard.cpp \
    src/sensorchartwidget.cpp \
    src/alertmanager.cpp \
    src/datalogger.cpp

HEADERS += \
    include/mainwindow.h \
    include/sensorworker.h \
    include/sensormodel.h \
    include/sensordashboard.h \
    include/sensorchartwidget.h \
    include/alertmanager.h \
    include/datalogger.h \
    include/sensordata.h

INCLUDEPATH += include

RESOURCES += resources/resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
