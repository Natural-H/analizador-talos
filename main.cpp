#include <QApplication>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    ui::MainWindow mainWindow;
    mainWindow.show();

    return QApplication::exec();
}
