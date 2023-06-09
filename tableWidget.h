#ifndef TABLE_WIDGET_H
#define TABLE_WIDGET_H

#include <QHeaderView>
#include <QList>
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include <QPrinter>
#include <QRegularExpression>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtWidgets>
#include <tuple>
#include <type_traits>
#include "tableWidget_global.h"

class TABLE_EXPORT HtmlPreviewWidget : public QPrintPreviewWidget {
   public:
    HtmlPreviewWidget(QString html)
        : htmlContent(html) {
        updatePreview();
    }

   protected:
    void paintEvent(QPaintEvent* event) override {
        QPrintPreviewWidget::paintEvent(event);

        QPainter painter(this);
        QTextDocument doc;

        doc.setHtml(htmlContent);
        doc.setDefaultTextOption(QTextOption(Qt::AlignLeft | Qt::AlignTop));
        doc.setPageSize(size());

        painter.save();
        painter.translate(rect().topLeft());

        // Draw the document's contents
        doc.drawContents(&painter);
        painter.restore();
    }

   private:
    QString htmlContent;
};

class TABLE_EXPORT CustomTableModel : public QStandardItemModel {
    Q_OBJECT

   public:
    explicit CustomTableModel(const QList<int>& editableColumns, const QList<int>& disabledColumns,
                              QObject* parent = nullptr)
        : QStandardItemModel(parent),
          editableColumns(editableColumns),
          disabledColumns(disabledColumns) {}

    Qt::ItemFlags flags(const QModelIndex& index) const override {
        if (!index.isValid())
            return Qt::NoItemFlags;

        if (editableColumns.contains(index.column()))
            return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable |
                                 Qt::ItemIsEnabled);  // Enable editing for the specified columns

        // Disable editing for the specified columns
        if (disabledColumns.contains(index.column()))
            return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        // Default behavior for other columns
        return QStandardItemModel::flags(index);
    }

   private:
    QList<int> editableColumns;
    QList<int> disabledColumns;
};

class TABLE_EXPORT TableWidget : public QTableView {
    Q_OBJECT

   public:
    QString title;
    QUrl logo;

    /**
     * Constructor for the TableWidget.
     */
    explicit TableWidget(QWidget* parent = nullptr, QList<int> editableColumns = QList<int>{},
                         QList<int> disabledColumns = QList<int>{})
        : QTableView(parent) {
        tableModel = new CustomTableModel(editableColumns, disabledColumns, this);

        proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(tableModel);
        proxyModel->setFilterKeyColumn(-1);
        setModel(proxyModel);

        // Set default properties
        setSelectionMode(QAbstractItemView::SingleSelection);
        setSelectionBehavior(QAbstractItemView::SelectRows);

        // Connect item selection signal
        connect(selectionModel(), &QItemSelectionModel::selectionChanged, this,
                &TableWidget::handleSelectionChanged);

        connect(model(), &QAbstractItemModel::dataChanged, this, &TableWidget::handleDataChanged,
                Qt::QueuedConnection);

        contextMenuEnabled = true;
        fit();
    }

    // Destructor
    ~TableWidget() {
        tableModel->deleteLater();
        proxyModel->deleteLater();
    }

    // Returns the number of rows
    int rowCount() const {
        return model()->rowCount();
    }

    // Returns the number of columns
    int columnCount() const {
        return model()->columnCount();
    }

    // Resize headers to fit content
    void fit() {
        // Set horizontal header resize mode to stretch for each column
        horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }

    // Resize headers, stretching them to fill parent
    void stretch() {
        // Set horizontal header resize mode to stretch for each column
        horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    // Set Interactive resizable headers
    void interactive() {
        horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    }

    // Sets the column to filter on. Default -1 (all columns)
    void setFilterKeyColumn(int column) {
        proxyModel->setFilterKeyColumn(column);
    }

    void setContextMenuEnabled(bool enabled) {
        contextMenuEnabled = enabled;
    }

    // Set table horizontal headers.
    // fieldNames should be equal in length to horizontalHeaders(otherwise won't be used)
    // fieldNames are used in generating JSON and CSV data.
    void setHorizontalHeaders(const QStringList& horizontalHeaders,
                              const QStringList& fieldNames_ = QStringList()) {
        // set headers
        headers = horizontalHeaders;

        // Set the horizontal Headers
        tableModel->setHorizontalHeaderLabels(horizontalHeaders);
        // Adjust header sizes to fit the contents
        horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        // Set the field names
        fieldNames = fieldNames_;
    }

    // fieldNames should be equal in length to horizontalHeaders(otherwise won't be used)
    // fieldNames are used in generating JSON and CSV data.
    void setFieldNames(const QStringList& fieldNames_) {
        fieldNames = fieldNames_;
    }

    /**
     * Sets vertical headers for the table.
     */
    void setVerticalHeaders(const QStringList& headers) {
        verticalHeaders = headers;
        if (!verticalHeaders.isEmpty())
            tableModel->setVerticalHeaderLabels(verticalHeaders);

        // Adjust header sizes to fit the contents
        if (!verticalHeaders.isEmpty())
            verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }

    void resetHeaders() {
        tableModel->setHorizontalHeaderLabels(headers);
        auto sectionResizeMode = horizontalHeader()->sectionResizeMode(0);
        horizontalHeader()->setSectionResizeMode(sectionResizeMode);

        if (!verticalHeaders.isEmpty()) {
            tableModel->setVerticalHeaderLabels(verticalHeaders);
            verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        }
    }

    /**
     * Populates the table with data.
     */
    void setData(const QVector<QStringList>& data) {
        tableModel->clear();
        tableModel->setRowCount(data.size());
        tableModel->setColumnCount(0);

        // Update the column count
        if (!data.isEmpty())
            tableModel->setColumnCount(data[0].size());

        // Update the headers because the table was cleared
        resetHeaders();

        for (int row = 0; row < data.size(); ++row) {
            const QStringList& rowDataList = data[row];
            for (int column = 0; column < rowDataList.size(); ++column) {
                auto item = new QStandardItem(rowDataList[column]);
                tableModel->setItem(row, column, item);
            }
        }
    }

    // Sets the signals and slots for double click on table. Calls handler with data for
    // the double-clicked row.
    void setDoubleClickHandler(std::function<void(int row, int col, const QStringList& data)> handler) {
        doubleClickHandler = std::move(handler);
    }

    // Generates an html table and writes it to a QString that is returned.
    QString generateHtmlTable() {
        QString html;

        int rowCount = model()->rowCount();
        int columnCount = model()->columnCount();

        // Start the HTML table with CSS styles
        html += "<table style='border-collapse: collapse; width: 100%;'>";

        // Generate table headers with CSS styles
        html += "<thead><tr>";
        for (int col = 0; col < columnCount; ++col) {
            html += "<th style='border: 1px solid #ddd; padding: 8px; background-color: #f2f2f2;'>";
            html += model()->headerData(col, Qt::Horizontal).toString();
            html += "</th>";
        }
        html += "</tr></thead>";

        // Generate table rows with CSS styles
        html += "<tbody>";
        for (int row = 0; row < rowCount; ++row) {
            html += "<tr>";
            for (int col = 0; col < columnCount; ++col) {
                html += "<td style='border: 1px solid #ddd; padding: 8px;'>";
                html += model()->data(model()->index(row, col)).toString();
                html += "</td>";
            }
            html += "</tr>";
        }
        html += "</tbody>";

        // End the HTML table
        html += "</table>";

        return html;
    }

    // Generates and returns QString containing CSV for the table data.
    QString generateCsvData() {
        QString csv;

        int rowCount = model()->rowCount();
        int columnCount = model()->columnCount();

        if (useFields()) {
            // Generate CSV headers
            for (int col = 0; col < columnCount; ++col) {
                if (col > 0) {
                    csv += ",";
                }
                csv += "\"" + fieldNames[col] + "\"";
            }
            csv += "\n";
        }

        // Generate CSV data rows
        for (int row = 0; row < rowCount; ++row) {
            for (int col = 0; col < columnCount; ++col) {
                if (col > 0) {
                    csv += ",";
                }
                QString value = model()->data(model()->index(row, col)).toString();
                // Quote the value if it contains a comma
                if (value.contains(',')) {
                    value = "\"" + value + "\"";
                }
                csv += value;
            }
            csv += "\n";
        }

        return csv;
    }

    // Generates and returns QString containing JSON for the table data.
    // The valueConverter is required if you want to convert cell data to other types from QString.
    QString generateJsonData(QVariant (*valueConverter)(int col, const QString& cellData) = nullptr) {
        QJsonArray rowsArray;

        int rowCount = model()->rowCount();
        int columnCount = model()->columnCount();

        auto useCustomFields = useFields();

        // Generate JSON data rows
        for (int row = 0; row < rowCount; ++row) {
            QJsonObject rowObject;
            for (int col = 0; col < columnCount; ++col) {
                QString columnName = model()->headerData(col, Qt::Horizontal).toString();

                if (useCustomFields) {
                    columnName = fieldNames[col];
                }

                QVariant cellValue = model()->data(model()->index(row, col));
                if (valueConverter) {
                    // Convert value using the user-provided callback function
                    cellValue = valueConverter(col, cellValue.toString());
                }
                rowObject[columnName] = QJsonValue::fromVariant(cellValue);
            }
            rowsArray.append(rowObject);
        }

        QJsonDocument jsonDoc(rowsArray);
        return jsonDoc.toJson();
    }

    void showPrintPreview() {
        // Generate the HTML table
        QString htmlTable = generateHtmlTable();

        QString html = "<div style=\"text-align: center; margin-bottom:16px;\">";

        if (!title.isEmpty()) {
            html += "<h1 style=\"font-size: 18px; margin-bottom: 4px;\">" + title + "</h1>";
        }

        if (!logo.isEmpty()) {
            html += QString("<div style=\"display: inline-block;\"><img src=\"%1\" width=\"64\" height=\"64\" /></div>").arg(logo.toString());
        }

        html += "<br/> </div>";
        html += htmlTable;

        // Create a QTextDocument to set the HTML content
        QTextDocument document;
        document.setHtml(html);

        // Create a custom HTML preview widget
        HtmlPreviewWidget previewWidget(html);

        // Create a QPrintPreviewDialog and set the custom preview widget
        QPrinter printer(QPrinter::HighResolution);
        QPrintPreviewDialog previewDialog(&printer);
        previewDialog.setMinimumSize(800, 600);
        previewDialog.setWindowTitle("Print Preview");

        // Set the custom preview widget as the central widget of the print preview dialog
        previewDialog.setWindowFlags(previewDialog.windowFlags() &
                                     ~Qt::WindowContextHelpButtonHint);

        QVBoxLayout* layout = new QVBoxLayout();
        layout->addWidget(&previewWidget);

        // Connect the paintRequested signal to handle the printing
        connect(&previewDialog, &QPrintPreviewDialog::paintRequested, this,
                [this, &document](QPrinter* printer) { document.print(printer); });

        // Show the print preview dialog
        previewDialog.exec();
    }

    void printTable(QPrinter* printer = nullptr) {
        // Create a QTextBrowser to display the HTML content
        QTextBrowser textBrowser;

        // Generate the HTML table from the QTableView
        QString htmlTable = generateHtmlTable();

        QString html = "<div style=\"text-align: center; margin-bottom:16px;\">";

        if (!title.isEmpty()) {
            html += "<h1 style=\"font-size: 18px; margin-bottom: 4px;\">" + title + "</h1>";
        }

        if (!logo.isEmpty()) {
            html += QString("<div style=\"display: inline-block;\"><img src=\"%1\" width=\"64\" height=\"64\" /></div>").arg(logo.toString());
        }

        html += "<br/> </div>";
        html += htmlTable;

        // Set the HTML content to the QTextBrowser
        textBrowser.setHtml(html);

        if (printer == nullptr) {
            printer = new QPrinter(QPrinter::HighResolution);
        }

        // Print the QTextBrowser content
        QPrintDialog printDialog(printer);
        if (printDialog.exec() == QDialog::Accepted) {
            textBrowser.print(printer);
        }
    }

    void appendRow(const QStringList& rowData) {
        int row = tableModel->rowCount();
        tableModel->setRowCount(row + 1);
        setRowData(row, rowData);
    }

    void deleteRow(int row) {
        if (row >= 0 && row < tableModel->rowCount()) {
            tableModel->removeRow(row);
        }
    }

    void clearTable() { tableModel->clear(); }

    void appendRows(const QVector<QStringList>& rowsData) {
        int currentRowCount = tableModel->rowCount();
        int rowsToAdd = rowsData.size();
        int newRowCount = currentRowCount + rowsToAdd;

        tableModel->setRowCount(newRowCount);

        for (int row = 0; row < rowsToAdd; ++row) {
            const QStringList& rowData = rowsData[row];
            setRowData(currentRowCount + row, rowData);
        }
    }

    auto getAllTableData() const {
        QList<QList<QVariant>> tableData;

        if (tableModel) {
            for (int row = 0; row < tableModel->rowCount(); ++row) {
                QList<QVariant> rowData;

                for (int col = 0; col < tableModel->columnCount(); ++col) {
                    QModelIndex index = tableModel->index(row, col);
                    QVariant data = tableModel->data(index);
                    rowData.append(data);
                }
                tableData.append(rowData);
            }
        }
        return tableData;
    }

    QList<QList<QString>> getSelectedRows() const {
        QList<QList<QString>> selectedRowsData;

        QModelIndexList selectedIndexes = selectionModel()->selectedRows();
        for (const QModelIndex& index : selectedIndexes) {
            QList<QString> rowData;

            for (int col = 0; col < tableModel->columnCount(); ++col) {
                QModelIndex dataIndex = tableModel->index(index.row(), col);
                QString data = tableModel->data(dataIndex).toString();
                rowData.append(data);
            }
            selectedRowsData.append(rowData);
        }

        return selectedRowsData;
    }

    std::optional<QStringList> getCurrentRow() const {
        std::optional<QStringList> rowData;

        QModelIndex index = currentIndex();
        if (index.isValid()) {
            rowData.emplace();

            for (int col = 0; col < tableModel->columnCount(); ++col) {
                QModelIndex dataIndex = tableModel->index(index.row(), col);
                QVariant data = tableModel->data(dataIndex);
                rowData->append(data.toString());
            }
        }
        return rowData;
    }

    void selectRowRange(int startRow, int endRow) {
        // Create the selection model and get the model index for the desired row
        QItemSelectionModel* selModel = selectionModel();
        // Get the model indices for the start and end rows
        QModelIndex startModelIndex = model()->index(startRow, 0);
        QModelIndex endModelIndex = model()->index(endRow, 0);

        // Create a selection range for the row range
        QItemSelection selection(startModelIndex, endModelIndex);

        // Clear the existing selection and select the desired row range
        selModel->clearSelection();
        selModel->select(selection, QItemSelectionModel::Select);
    }

   protected:
    void keyPressEvent(QKeyEvent* event) override {

        // Check if Ctrl+Shift+P is pressed
        if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier) &&
            event->key() == Qt::Key_P) {
            // Show print preview
            showPrintPreview();
            return;
        } else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_P) {
            printTable();
            return;
        }
        QTableView::keyPressEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent* event) override {
        QModelIndex index = indexAt(event->pos());
        if (index.isValid()) {
            int row = index.row();
            int column = index.column();

            QStringList rowData;
            for (int c = 0; c < tableModel->columnCount(); ++c) {
                auto cellItem = tableModel->item(row, c);
                if (cellItem)
                    rowData.append(cellItem->text());
            }
            doubleClickHandler(row, column, rowData);
        }

        // Call base class implementation
        QTableView::mouseDoubleClickEvent(event);
    }

    void contextMenuEvent(QContextMenuEvent* event) override {
        if (!contextMenuEnabled)
            return;

        QMenu contextMenu(this);

        // Add actions to the context menu
        QAction* copyAction = contextMenu.addAction("Copy");
        QAction* pasteAction = contextMenu.addAction("Paste");
        QAction* deleteAction = contextMenu.addAction("Remove");

        // Add more actions as needed

        // Show the context menu at the mouse position
        QPoint pos = event->globalPos();
        QAction* selectedItem = contextMenu.exec(mapToGlobal(pos));

        // Handle the selected action
        if (selectedItem == copyAction) {
            // Perform copy action
            std::optional<QStringList> rowData = getCurrentRow();
            if (rowData) {
                QApplication::clipboard()->setText(rowData->join("\t"));
            }
        } else if (selectedItem == pasteAction) {
            QString clipboardText = QApplication::clipboard()->text();
            if (clipboardText.isEmpty())
                return;

            QStringList items = clipboardText.split("\t");
            if (items.size() == model()->columnCount()) {
                appendRow(items);
            }
        } else if (selectedItem == deleteAction) {
            QModelIndex index = currentIndex();
            if (index.isValid()) {
                deleteRow(index.row());
            }
        }

        QTableView::contextMenuEvent(event);
    }

   signals:
    void tableSelectionChanged(int row, int column, const QStringList& rowData);
    void rowUpdated(int row, int column, const QStringList& rowData);

   public slots:
    void filterTable(const QString& query,
                     const QRegularExpression::PatternOption caseSensitivity = QRegularExpression::CaseInsensitiveOption,
                     int column = -1) {

        if (query.isEmpty()) {
            proxyModel->invalidate();
            return;
        }

        // -1 is all columns
        if (column >= -1 && column < model()->columnCount()) {
            proxyModel->setFilterKeyColumn(column);
        }
        QRegularExpression regex(query, caseSensitivity);
        proxyModel->setFilterRegularExpression(regex);
    }

   private slots:
    void handleSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
        Q_UNUSED(deselected);
        if (selected.isEmpty())
            return;

        int selectedRow = selected.indexes().first().row();
        int selectedCol = selected.indexes().first().column();

        QStringList rowData;
        for (int column = 0; column < tableModel->columnCount(); ++column) {
            auto item = tableModel->item(selectedRow, column);
            if (item)
                rowData.append(item->text());
        }
        emit tableSelectionChanged(selectedRow, selectedCol, rowData);
    }

    void handleDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                           const QVector<int>& roles = QVector<int>()) {
        Q_UNUSED(roles);

        // If no row selected, we are just setting the data
        if (selectionModel()->selectedIndexes().isEmpty())
            return;

        if (topLeft.row() != bottomRight.row()) {
            // Only handle single-row changes
            return;
        }

        QStringList rowData;
        const int row = topLeft.row();

        for (int column = 0; column < tableModel->columnCount(); ++column) {
            QModelIndex index = tableModel->index(row, column);
            QVariant data = tableModel->data(index);
            QString cellData = data.toString();
            rowData.append(cellData);
        }
        emit rowUpdated(row, topLeft.column(), rowData);
        return QTableView::dataChanged(topLeft, bottomRight, roles);
    }

   private:
    std::function<void(int, int, const QStringList&)> doubleClickHandler;
    void setRowData(int row, const QStringList& rowData) {
        for (int column = 0; column < tableModel->columnCount(); ++column) {
            QStandardItem* item = new QStandardItem();
            item->setText(rowData.value(column));
            tableModel->setItem(row, column, item);
        }
    }

    bool contextMenuEnabled;

    // Initialize the table model
    CustomTableModel* tableModel;

    // Initialize QSortFilterProxy table model to filter the table.
    QSortFilterProxyModel* proxyModel;

    // Table Headers
    // e.g ["ID", "First Name", "Created At"]
    QStringList headers;

    // e.g ["id", "first_name", "created_at"] used in generating json & csv data
    QStringList fieldNames;

    // Vertical Headers
    QStringList verticalHeaders;

    // use fieldNames in generating csv and json
    bool useFields() const {
        return (headers.size() == fieldNames.size()) && (fieldNames.size() == model()->columnCount());
    }

    void handleValidationError(int row, int column) {
        QModelIndex index = model()->index(row, column);
        if (index.isValid()) {
            // Set the item's background color or display an error message
            // to indicate the validation error
            QVariant backgroundColor = QColor(Qt::red);
            model()->setData(index, backgroundColor, Qt::BackgroundRole);
        }
    }
};

#endif  // TABLE_WIDGET_H
