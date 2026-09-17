#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QFile stylesheet(":/styles.qss");
    if (stylesheet.open(QFile::ReadOnly | QFile::Text))
    {
        QString style = QLatin1String(stylesheet.readAll());
        a.setStyleSheet(style);
    }

    w.show();
    return QApplication::exec();
}
