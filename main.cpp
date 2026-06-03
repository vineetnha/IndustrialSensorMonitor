#include <QApplication>
#include <QStyleFactory>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Apply Fusion style for a modern cross-platform look
    app.setStyle(QStyleFactory::create("Fusion"));

    // Dark palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window,          QColor(44, 62, 80));
    darkPalette.setColor(QPalette::WindowText,       Qt::white);
    darkPalette.setColor(QPalette::Base,             QColor(52, 73, 94));
    darkPalette.setColor(QPalette::AlternateBase,    QColor(57, 79, 105));
    darkPalette.setColor(QPalette::ToolTipBase,      Qt::white);
    darkPalette.setColor(QPalette::ToolTipText,      Qt::white);
    darkPalette.setColor(QPalette::Text,             Qt::white);
    darkPalette.setColor(QPalette::Button,           QColor(44, 62, 80));
    darkPalette.setColor(QPalette::ButtonText,       Qt::white);
    darkPalette.setColor(QPalette::BrightText,       Qt::red);
    darkPalette.setColor(QPalette::Link,             QColor(52, 152, 219));
    darkPalette.setColor(QPalette::Highlight,        QColor(52, 152, 219));
    darkPalette.setColor(QPalette::HighlightedText,  Qt::white);
    app.setPalette(darkPalette);

    app.setApplicationName("Industrial Sensor Monitor");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("PortfolioProject");

    MainWindow window;
    window.show();

    return app.exec();
}
