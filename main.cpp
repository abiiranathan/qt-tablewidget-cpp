#include <QApplication>
#include "mainwindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationDisplayName("Application");
    app.setOrganizationName("com.yomedicalfile.com");
    app.setApplicationName("AppName");

    MainWindow w;
    w.show();
    return app.exec();
}
