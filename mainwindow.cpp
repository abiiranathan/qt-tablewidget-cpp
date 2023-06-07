#include "mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <iostream>
#include "delegates.h"
#include "tableWidget.h"
#include "ui_mainwindow.h"

void writeQStringToFile(const QString& str, const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qInfo() << Q_FUNC_INFO << "Could not open file for writing: " << fileName;
        return;
    }

    QTextStream stream(&file);
    stream << str;
}

QVariant valueConverter(int col, const QString& value) {
    switch (col) {
        case 0:
            return value.toInt();
        default:
            return value;
    }
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    TableWidget* table = new TableWidget(this, QList<int>(), QList<int>{0, 1});

    table->setHorizontalHeaders(QStringList{"ID", "Name", "DOB", "Sex", "CreatedAt", "Time"},
                                QStringList{"id", "name", "dob", "sex", "created_at", "time"});

    table->setItemDelegateForColumn(2, new DateDelegate(this));
    table->setItemDelegateForColumn(3, new ComboBoxDelegate(this, QStringList{"Male", "Female"}));
    table->setItemDelegateForColumn(4, new DateTimeDelegate(this));
    table->setItemDelegateForColumn(5, new TimeDelegate(this));

    table->setRows(
        QVector<QStringList>{{"1", "Abiira Nathan", "1989-05-18", "Male", "2023-06-07T06:30:13.075Z", "16:30:34"},
                             {"2", "Kwikiriza Dan", "2005-06-12", "Female", "null", "00:30:00"}});

    connect(table, &TableWidget::tableSelectionChanged, this, [](int rows, int cols, QList<QString> rowData) {
        std::cout << "Selection changed" << std::endl;
    });

    table->setDoubleClickHandler(
        [](int row, int col, const QList<QString>& rowData) { std::cout << "doubleclick handler" << std::endl; });

    connect(table, &TableWidget::rowUpdated, this, [](int row, int col, const QList<QString>& rowData) {
        std::cout << "rowUpdated" << std::endl;
    });
    setCentralWidget(table);

    writeQStringToFile(table->generateCsvData(), "data.csv");
    writeQStringToFile(table->generateJsonData(&valueConverter), "data.json");
}

MainWindow::~MainWindow() {
    delete ui;
}
